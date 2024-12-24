#pragma once

#include "detectSection.hpp"
#include "timeUtil.hpp"

// 检测流任务类型
enum class DetectStreamWorkType{
    START_TASK = 0,
    ADD_IMAGE,
    END_TASK
};

// 检测流任务
class DetectStreamWork{
public:
    // 任务类型
    DetectStreamWorkType type;
    // 顺向图像
    cv::Mat forwardImage;
    // 逆向图像
    cv::Mat backwardImage;

    // 构造函数
    DetectStreamWork() {};
    // 构造函数
    // @param type: 任务类型
    DetectStreamWork(const DetectStreamWorkType type) :
        type(type)
    {
    };
    // 构造函数
    // @param type: 任务类型
    // @param forwardImage: 顺向图像
    // @param backwardImage: 逆向图像
    DetectStreamWork(DetectStreamWorkType type, const cv::Mat& forwardImage, const cv::Mat& backwardImage) :
        type(type),
        forwardImage(forwardImage),
        backwardImage(backwardImage)
    {
    };
};

// 检测流, 是对检测截面的封装，提供一个消息队列，避免考虑等待检测截面完成上一根角钢的问题
// T为OutputFrameinterface的实现
template <typename T, typename = std::enable_if_t<std::is_base_of_v<OutputFrameinterface, T>>>
class DetectStream{
private:
    // 检测截面
    DetectSection<T> detectSection;

    // 检测流任务队列
    std::queue<DetectStreamWork> workQueue;
    // 检测流任务队列锁
    std::mutex workMutex;
    // 检测流任务队列条件变量
    std::condition_variable workCond;
    // 运行状态
    bool running = false;

    // 检测截面状态
    bool sectionWorking = false;
    // 检测截面状态锁
    std::mutex sectionMutex;
    // 检测截面状态条件变量
    std::condition_variable sectionCond;

    // 结果队列
    std::queue<DetectSectionWork<T>> resultQueue;
    // 结果队列锁
    std::mutex resultMutex;
    // 结果队列条件变量
    std::condition_variable resultCond;

    // 工作线程
    std::thread worker;
    // 回调
    std::function<void(cv::Mat, T)> callback;
    // 内部回调
    std::function<void(cv::Mat, T)> innerCallback;
    // 工作循环
    void WorkLoop() {
        while(running){
            DetectStreamWork work;
            {
                std::unique_lock<std::mutex> lock(workMutex);
                workCond.wait(lock, [this]{
                    return !workQueue.empty() || !running;
                });
                if(!running && workQueue.empty()){
                    break;
                }
                work = workQueue.front();
                workQueue.pop();
            }

            switch(work.type){
                case DetectStreamWorkType::START_TASK:
                {
                    // 需要等待上一个任务完成
                    std::unique_lock<std::mutex> lock(sectionMutex);
                    if(sectionWorking){
                        sectionCond.wait(lock, [this]{
                            return !sectionWorking || !running;
                        });
                    }
                    if(!running){
                        break;
                    }
                    sectionWorking = true;
                    detectSection.StartTask(innerCallback);
                    break;
                }
                case DetectStreamWorkType::ADD_IMAGE:
                {
                    detectSection.AddImage(work.forwardImage, work.backwardImage);
                    break;
                }
                case DetectStreamWorkType::END_TASK:
                {
                    detectSection.EndTask();
                    break;
                }
                default:
                    break;
            }
        }
    };

public:
    // 构造函数
    // @param detector: 检测器
    // @param sampleInterval: 采样间隔
    // @param onResult: 结果回调
    DetectStream(
        DetectorInterface<T>& detector, 
        int sampleInterval,
        std::function<void(cv::Mat, T)> onResult
    ) :
        detectSection(detector, sampleInterval),
        callback(onResult)
    {
        innerCallback = [this](cv::Mat img, T output){
            this->callback(img, output);
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                resultQueue.push(DetectSectionWork<T>(img, output));
            }
            resultCond.notify_one();
            {
                std::lock_guard<std::mutex> lock(sectionMutex);
                sectionWorking = false;
            }
            sectionCond.notify_one();
        };
    };

    // 复制构造函数
    DetectStream(const DetectStream<T>& detectStream) = delete;

    // 赋值构造函数
    DetectStream<T>& operator=(const DetectStream<T>& detectStream) = delete;

    // 移动构造函数
    DetectStream(DetectStream<T>&& detectStream) :
        detectSection(std::move(detectStream.detectSection)),
        callback(detectStream.callback),
        running(false)
    {
        // 重新设置启动一下
        if(detectStream.running){
            Start();
        }
        this->innerCallback = [this](cv::Mat img, T output){
            this->callback(img, output);
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                resultQueue.push(DetectSectionWork<T>(img, output));
            }
            resultCond.notify_one();
            {
                std::lock_guard<std::mutex> lock(sectionMutex);
                sectionWorking = false;
            }
            sectionCond.notify_one();
        };
    };

    // 析构函数
    ~DetectStream() {
        Stop();
    };

    // 开始
    // @return 是否成功
    bool Start() {
        if(running){
            Log_Warning("DetectStream is already running");
            return true;
        }
        if(detectSection.CheckState() != DetectSectionState::AVAILABLE){
            if(detectSection.CheckState() == DetectSectionState::INIT){
                if(!detectSection.Start()){
                    Log_Error("DetectSection start failed");
                    return false;
                }
            }
            else{
                Log_Error("DetectSection is not available");
                return false;
            }
        }
        running = true;
        worker = std::thread(&DetectStream::WorkLoop, this);
        return true;
    };

    // 停止
    void Stop() {
        if(running){
            running = false;
            detectSection.Stop();
            sectionCond.notify_one();
            workCond.notify_one();
            worker.join();
        }
    };

    // 开始一个新的角钢
    // @return 是否成功
    bool StartTask() {
        if(!running){
            Log_Error("DetectStream is not running");
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(workMutex);
            workQueue.push(DetectStreamWork(DetectStreamWorkType::START_TASK));
        }
        workCond.notify_one();
        return true;
    };

    // 添加一帧图像
    // @param forwardImage: 顺向图像
    // @param backwardImage: 逆向图像
    // @return 是否成功
    bool AddImage(const cv::Mat& forwardImage, const cv::Mat& backwardImage) {
        if(!running){
            Log_Error("DetectStream is not running");
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(workMutex);
            workQueue.push(DetectStreamWork(DetectStreamWorkType::ADD_IMAGE, forwardImage, backwardImage));
        }
        workCond.notify_one();
        return true;
    };

    // 结束一个角钢
    // @return 是否成功
    bool EndTask() {
        if(!running){
            Log_Error("DetectStream is not running");
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(workMutex);
            workQueue.push(DetectStreamWork(DetectStreamWorkType::END_TASK));
        }
        workCond.notify_one();
        return true;
    };

    // 获取结果, 不会等待
    // @return 结果
    std::optional<DetectSectionWork<T>> GetResult() {
        std::unique_lock<std::mutex> lock(resultMutex);
        if(resultQueue.empty()){
            return std::nullopt;
        }
        DetectSectionWork<T> ret = resultQueue.front();
        resultQueue.pop();
        return ret;
    };

    // 获取结果, 等待
    // @return 结果
    std::optional<DetectSectionWork<T>> WaitResult() {
        std::unique_lock<std::mutex> lock(resultMutex);
        resultCond.wait(lock, [this]{
            return !resultQueue.empty();
        });
        DetectSectionWork<T> ret = resultQueue.front();
        resultQueue.pop();
        return ret;
    };
};
