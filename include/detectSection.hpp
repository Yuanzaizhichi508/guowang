#pragma once

#include "detectorInterface.h"
#include "threadPool.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <queue>
#include <atomic>

#define DETECT_SECTION_MAX_VECTOR_SIZE 256
#define DETECT_THREAD_POOL_SIZE 4

// 检测截面状态
enum class DetectSectionState {
    INIT,       // 初始化
    AVAILABLE,  // 可用
    WORKING,    // 工作中
    WAITING,    // 等待
    ERR         // 错误
};

// 检测截面任务
// T为OutputFrameinterface的实现
template <typename T, typename = std::enable_if_t<std::is_base_of_v<OutputFrameinterface, T>>>
class DetectSectionWork {
public:
    // 输入图像
    cv::Mat image;

    // 输出结果
    T output;

    // 构造函数
    DetectSectionWork() {};
    // 构造函数
    // @param image: 输入图像
    DetectSectionWork(const cv::Mat& image): image(image) {};
    // 构造函数
    // @param image: 输入图像
    // @param output: 输出结果
    DetectSectionWork(const cv::Mat& image, const T& output): image(image), output(output) {};
};

// 检测截面线程池任务类型
enum class DetectSectionThreadPoolWorkType {
    // 检测位置
    POSITION,
    // 检查并识别
    CHECK_AND_STRING
};

// 检测截面线程池任务
class DetectSectionThreadPoolWork {
public:
    // 帧索引
    int workIndex;

    // 是否为顺向
    bool isForward;

    // 任务类型
    DetectSectionThreadPoolWorkType type;

    // 构造函数
    DetectSectionThreadPoolWork() {};
    // 构造函数
    // @param workIndex: 帧索引
    // @param isForward: 是否为顺向
    // @param type: 任务类型
    DetectSectionThreadPoolWork(
        int workIndex, 
        bool isForward, 
        DetectSectionThreadPoolWorkType type
    ) :
        workIndex(workIndex),
        isForward(isForward),
        type(type)
    {
    };
};

// 检测截面
// T为OutputFrameinterface的实现
template <typename T, typename = std::enable_if_t<std::is_base_of_v<OutputFrameinterface, T>>>
class DetectSection {
private:
    // 检测器
    DetectorInterface<T>& detector;

    // 截面状态
    DetectSectionState state = DetectSectionState::INIT;

    // 截面运行状态
    bool running = false;

    // 任务序列，一帧包含顺向和逆向两张图片
    // 顺向图片在前，逆向图片在后
    // 顺向指画面相对视野由右向左移动，逆向指画面相对视野由左向右移动
    // 使用固定size的vector，避免频繁resize
    std::vector<std::pair<DetectSectionWork<T>, DetectSectionWork<T>>> workVector;
    // 任务队列，存储任务索引
    std::queue<int> workQueue;

    // 采样间隔
    int sampleInterval;
    // 上次采样索引
    int lastSampleIndex = 0;
    // 下次添加索引
    int nextAddIndex = 0;
    // 上次加入任务队列索引
    int lastPushIndex = -1;
    // 上次检测位置
    std::pair<PositionType, PositionType> lastSamplePosition = {PositionType::NONE, PositionType::NONE};


    // 主工作线程
    std::thread worker;
    // 主工作线程锁
    std::mutex workMutex;
    // 主工作线程条件变量
    std::condition_variable workCond;
    // 回调函数
    std::optional<std::function<void(cv::Mat, T)>> callback;
    // 主工作循环
    void WorkLoop(){
        while(this->running){
            // 首先获取图片
            int sampleIndex = -1;
            bool endPhase = false;
            {
                std::unique_lock<std::mutex> lock(this->workMutex);
                this->workCond.wait(lock, [this]{
                    return 
                    (
                        !this->workQueue.empty() && 
                        (this->state == DetectSectionState::WORKING || this->state == DetectSectionState::WAITING)
                    ) ||
                    !this->running;
                });
                // AVAILABE状态代表这一阶段已经结束
                if(!this->running){
                    break;
                }
                sampleIndex = this->workQueue.front();
                this->workQueue.pop();
                if(this->workQueue.empty()){
                    // 如果是WAIT状态，且队列为空，那么说明这一阶段已经结束
                    if(this->state == DetectSectionState::WAITING){
                        endPhase = true;
                    }
                }
            }
            if(sampleIndex == -1){
                // 无法获取图片
                Log_Error("sampleIndex == -1");
                continue;
            }
            // 处理
            // 首先检测位置
            DetectSectionWork<T>& forward = this->workVector[sampleIndex].first;
            DetectSectionWork<T>& backward = this->workVector[sampleIndex].second;
            DetectSectionThreadPoolWork workF(sampleIndex, true, DetectSectionThreadPoolWorkType::POSITION);
            DetectSectionThreadPoolWork workB(sampleIndex, false, DetectSectionThreadPoolWorkType::POSITION);
            std::vector<DetectSectionThreadPoolWork> works = {workF, workB};
            SyncWaitThreadPoolDone(works);
        
            bool forwardFound = false;
            bool backwardFound = false;
            int startIdx = -1;
            int endIdx = -1;

            // 判断位置
            this->CheckPosition(forward, backward, forwardFound, backwardFound, startIdx, endIdx, sampleIndex);

            this->lastSampleIndex = sampleIndex;
            this->lastSamplePosition = {forward.output.position, backward.output.position};

            // 处理搜索范围内的图片
            if(forwardFound && backwardFound){
                // 理论上不会出现这样的情况
                Log_Error("forwardFound && backwardFound");
                // 两边都识别一次，返回结果较长的
                int forwardIdx = this->SearchRange(startIdx, endIdx, true);
                int backwardIdx = this->SearchRange(startIdx, endIdx, false);
                if(forwardIdx != -1 && backwardIdx != -1){
                    DetectSectionWork<T>& forwardWork = this->workVector[forwardIdx].first;
                    DetectSectionWork<T>& backwardWork = this->workVector[backwardIdx].second;
                    if(forwardWork.output.result.size() > backwardWork.output.result.size()){
                        if(this->callback.has_value()){
                            this->callback.value()(forwardWork.image, forwardWork.output);
                        }
                        else{
                            Log_Error("callback is not set");
                        }
                    }
                    else{
                        if(this->callback.has_value()){
                            this->callback.value()(backwardWork.image, backwardWork.output);
                        }
                        else{
                            Log_Error("callback is not set");
                        }
                    }
                }
                else if(forwardIdx != -1){
                    DetectSectionWork<T>& forwardWork = this->workVector[forwardIdx].first;
                    if(this->callback.has_value()){
                        this->callback.value()(forwardWork.image, forwardWork.output);
                    }
                    else{
                        Log_Error("callback is not set");
                    }
                }
                else if(backwardIdx != -1){
                    DetectSectionWork<T>& backwardWork = this->workVector[backwardIdx].second;
                    if(this->callback.has_value()){
                        this->callback.value()(backwardWork.image, backwardWork.output);
                    }
                    else{
                        Log_Error("callback is not set");
                    }
                }
                else{
                    // 在搜索范围内没有找到
                    Log_Error("forwardIdx == -1 && backwardIdx == -1");
                    this->EmptyEnd();
                }
                this->Found();
            }
            else if(forwardFound){
                Log_Debug("forwardFound");
                int forwardIdx = this->SearchRange(startIdx, endIdx, true);
                if(forwardIdx != -1){
                    DetectSectionWork<T>& work = this->workVector[forwardIdx].first;
                    if(this->callback.has_value()){
                        this->callback.value()(work.image, work.output);
                    }
                    else{
                        Log_Error("callback is not set");
                    }
                }
                else{
                    // 在搜索范围内没有找到
                    Log_Error("forwardIdx == -1");
                    this->EmptyEnd();
                }
                this->Found();
            }
            else if(backwardFound){
                Log_Debug("backwardFound");
                int backwardIdx = this->SearchRange(startIdx, endIdx, false);
                if(backwardIdx != -1){
                    DetectSectionWork<T>& work = this->workVector[backwardIdx].second;
                    if(this->callback.has_value()){
                        this->callback.value()(work.image, work.output);
                    }
                    else{
                        Log_Error("callback is not set");
                    }
                }
                else{
                    // 在搜索范围内没有找到
                    Log_Error("backwardIdx == -1");
                    this->EmptyEnd();
                }
                this->Found();
            }
            else{
                // 未找到
                if(endPhase){
                    Log_Debug("Not found at end phase");
                    this->EmptyEnd();
                    this->Found();
                }
            }
        }
    };

    // 线程池
    ThreadPool threadPool;
    // 线程池工作函数
    // @param work: 线程池任务
    // @return 是否成功
    bool ThreadPoolWorkFunc(const DetectSectionThreadPoolWork& work){
        DetectSectionWork<T>& detectWork = work.isForward ? this->workVector[work.workIndex].first : this->workVector[work.workIndex].second;
        bool success = true;
        switch(work.type){
            case DetectSectionThreadPoolWorkType::POSITION:
            {
                if(!this->detector.DetectPosition(detectWork.image, detectWork.output)) {
                    Log_Error("ThreadPoolPositionWork failed");
                    success = false;
                }
                break;
            }
            case DetectSectionThreadPoolWorkType::CHECK_AND_STRING:
            {
                if(!((detectWork.output.state >> 1) & 1)){
                    success = this->detector.DetectPosition(detectWork.image, detectWork.output);
                }
                if(detectWork.output.position == PositionType::FINE && !((detectWork.output.state >> 2) & 1)){
                    success &= this->detector._ExtractString(detectWork.output, detectWork.image);
                }
                if(!success){
                    Log_Error("ThreadPoolCheckAndStringWork failed");
                }
                break;
            }
            default:
                break;
        }
        return success;
    };

    // 同步等待线程池完成
    // @param works: 线程池任务
    inline void SyncWaitThreadPoolDone(const std::vector<DetectSectionThreadPoolWork>& works) {
        std::vector<std::future<bool>> futures;
        for(auto& work : works){
            futures.push_back(this->threadPool.addTask(
                std::bind(&DetectSection::ThreadPoolWorkFunc, this, work)
            ));
        }
        for(auto& future : futures){
            future.get();
        }
    };

    // 检查检测的位置
    // @param forward: 顺向任务
    // @param backward: 逆向任务
    // @param forwardFound: 顺向是否找到(输出)
    // @param backwardFound: 逆向是否找到(输出)
    // @param startIdx: 开始索引(输出)
    // @param endIdx: 结束索引(输出)
    // @param sampleIndex: 采样索引
    inline void CheckPosition(
        const DetectSectionWork<T>& forward,
        const DetectSectionWork<T>& backward,
        bool& forwardFound,
        bool& backwardFound,
        int& startIdx,
        int& endIdx,
        int sampleIndex
    ) {
        Log_Debug("CheckPosition @ " + std::to_string(sampleIndex));
        // 检查位置
        switch(forward.output.position)
        {
            case PositionType::NONE:
                switch(lastSamplePosition.first)
                {
                    case PositionType::NONE:
                        // 无事发生
                        break;
                    case PositionType::LEFT:
                        // 如果是LEFT，那么应该之前就处理了，这里是错误
                        Log_Error("Error: LAST FORWARD == LEFT && CUR FORWARD == NONE");
                        break;
                    case PositionType::RIGHT:
                        // 如果是RIGHT, 实际上此时理论上如果没有误差的话应该不会是None，不过实际情况会比较复杂，留给后续处理
                        forwardFound = true;
                        startIdx = lastSampleIndex + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::FINE:
                        // 如果是FINE，那么应该之前就处理了，这里是错误
                        Log_Error("Error: LAST FORWARD == FINE  && CUR FORWARD == NONE");
                        break;
                    default:
                        break;
                }
                break;
            case PositionType::FINE:
                // 找到了
                forwardFound = true;
                startIdx = sampleIndex;
                endIdx = sampleIndex;
                break;
            case PositionType::LEFT:
                // 如果是LEFT，那么搜索范围应该是上一次的位置到当前位置
                switch(lastSamplePosition.first)
                {
                    case PositionType::NONE:
                        // 如果是LEFT，理论上是不会出现的，不过实际情况会比较复杂，留给后续处理
                        forwardFound = true;
                        startIdx = lastSampleIndex + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::LEFT:
                        // 这种情况肯定是错误的
                        Log_Error("Error: LAST FORWARD == LEFT && CUR FORWARD == LEFT");
                        break;
                    case PositionType::RIGHT:
                        // 如果是RIGHT，那么搜索范围在上一次的位置到当前位置
                        forwardFound = true;
                        startIdx = lastSampleIndex + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::FINE:
                        // 如果是FINE，那么之前就处理了，这里是错误
                        Log_Error("Error: LAST FORWARD == FINE  && CUR FORWARD == LEFT");
                        break;
                    default:
                        break;
                }
                break;
            case PositionType::RIGHT:
                // 如果是RIGHT，那么搜索范围应该是此次位置到下一次的位置
                break;
            default:
                break;
        }
        switch(backward.output.position)
        {
            case PositionType::NONE:
                switch(lastSamplePosition.second)
                {
                    case PositionType::NONE:
                        // 无事发生
                        break;
                    case PositionType::LEFT:
                        // 如果是LEFT，实际上此时理论上如果没有误差的话应该不会是None，不过实际情况会比较复杂，留给后续处理
                        backwardFound = true;
                        startIdx = lastSampleIndex + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::RIGHT:
                        // 如果是RIGHT，那么应该之前就处理了，这里是错误
                        Log_Error("Error: LAST BACKWARD == RIGHT && CUR BACKWARD == NONE");
                        break;
                    case PositionType::FINE:
                        // 如果是FINE，那么应该之前就处理了，这里是错误
                        Log_Error("Error: LAST BACKWARD == FINE  && CUR BACKWARD == NONE");
                        break;
                    default:
                        break;
                }
                break;
            case PositionType::FINE:
                // 找到了
                backwardFound = true;
                startIdx = sampleIndex;
                endIdx = sampleIndex;
                break;
            case PositionType::LEFT:
                // 如果是LEFT，那么搜索范围应该是此次位置到下一次的位置
                break;
            case PositionType::RIGHT:
                // 如果是RIGHT，那么搜索范围应该是上一次的位置到当前位置
                switch(lastSamplePosition.second)
                {
                    case PositionType::NONE:
                        // 如果是RIGHT，理论上是不会出现的，不过实际情况会比较复杂，留给后续处理
                        backwardFound = true;
                        startIdx = lastSampleIndex  + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::LEFT:
                        // 如果是LEFT，那么搜索范围在上一次的位置到当前位置
                        backwardFound = true;
                        startIdx = lastSampleIndex  + 1;
                        endIdx = sampleIndex - 1;
                        break;
                    case PositionType::RIGHT:
                        // 这种情况肯定是错误的
                        Log_Error("Error: LAST BACKWARD == RIGHT && CUR BACKWARD == RIGHT");
                        break;
                    case PositionType::FINE:
                        // 如果是FINE，那么之前就处理了，这里是错误
                        Log_Error("Error: LAST BACKWARD == FINE  && CUR BACKWARD == RIGHT");
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    };

    // 搜索范围内的目标，返回目标位置
    // @param start: 开始索引
    // @param end: 结束索引
    // @param forward: 是否为顺向
    // @return 目标位置, -1表示未找到
    inline int SearchRange(int start, int end, bool isForward) {
        Log_Info("SearchRange: " + std::to_string(start) + " - " + std::to_string(end) + " @ " + (isForward ? "Forward" : "Backward"));
        if(start < 0 || end >= this->workVector.size() || start > end){
            Log_Error("Error: SearchRange Out of Range");
            return -1;
        }
        // 使用线程数最多为sampleInterval-1
        std::vector<DetectSectionThreadPoolWork> works;
        for(int i = start; i <= end; ++i){
            works.push_back(DetectSectionThreadPoolWork(i, isForward, DetectSectionThreadPoolWorkType::CHECK_AND_STRING));
        }
        SyncWaitThreadPoolDone(works);

        for(int i = start; i <= end; ++i){
            auto& [forward, backward] = this->workVector[i];
            auto& work = isForward ? forward : backward;
            if(work.output.position == PositionType::FINE){
                return i;
            }
        }
        return -1;
    };

    // 找到之后的操作
    inline void Found() {
        {
            std::unique_lock<std::mutex> lock(this->workMutex);
            this->state = DetectSectionState::AVAILABLE;
            this->callback.reset();
        }
    };

    // 未找到之后的操作
    inline void EmptyEnd() {
        Log_Info("EmptyEnd");
        auto& [forward, backward] = this->workVector[this->nextAddIndex - 1];
        if(this->callback.has_value()){
            this->callback.value()(forward.image, forward.output);
        }
        else{
            Log_Error("callback is not set");
        }
    };

public:
    // 构造函数
    // @param detector: 检测器
    // @param sampleInterval: 采样间隔
    DetectSection(
        DetectorInterface<T>& detector, 
        int sampleInterval = 3
    ) : 
        detector(detector), 
        sampleInterval(sampleInterval),
        threadPool(DETECT_THREAD_POOL_SIZE)
    {
    };

    // 复制构造函数
    DetectSection(const DetectSection<T>& detectSection) = delete;

    // 赋值构造函数
    DetectSection<T>& operator=(const DetectSection<T>& detectSection) = delete;

    // 移动构造函数, 注意并不会保留大部分状态，移动之后相当于重新初始化
    DetectSection(DetectSection<T>&& detectSection) :
        detector(detectSection.detector),
        sampleInterval(detectSection.sampleInterval),
        running(false),
        state(DetectSectionState::INIT),
        threadPool(DETECT_THREAD_POOL_SIZE)
    {
        Log_Debug("DetectSection Move Constructor");
    };

    // 析构函数
    ~DetectSection() {
        Log_Debug("DetectSection Destructor");
        Stop();
    };

    // 启动
    // @return 是否成功
    bool Start() {
        if(!this->detector.CheckAvailable()){
            Log_Error("Detector is not available");
            return false;
        }
        if(this->running){
            Log_Error("DetectSection is already running");
            return false;
        }

        this->worker = std::thread(&DetectSection::WorkLoop, this);
        this->state = DetectSectionState::AVAILABLE;
        this->running = true;
        Log_Debug("DetectSection Start");
        return true;
    };

    // 停止
    void Stop() {
        if(this->running){
            this->running = false;
            this->threadPool.stop();
            this->workCond.notify_one();
            this->worker.join();
        }
    };

    // 检查状态
    // @return 状态
    DetectSectionState CheckState() const {
        return this->state;
    };

    // 开始一个新的角钢
    // @param callback: 回调函数
    // @return 是否成功
    bool StartTask(std::function<void(cv::Mat, T)> callback) {
#ifdef DEBUG
        std::cout << "StartTask\n";
#endif
        if(this->callback.has_value()){
            Log_Error("callback has value");
            return false;
        }
        if(!this->running){
            Log_Error("DetectSection is not running");
            return false;
        }
        if(this->state != DetectSectionState::AVAILABLE){
            Log_Error("DetectSection is not AVAILABLE");
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(this->workMutex);
            this->callback = callback;
            this->state = DetectSectionState::WORKING;
            this->workVector.resize(DETECT_SECTION_MAX_VECTOR_SIZE);
            this->workQueue = std::queue<int>();
            this->lastSampleIndex = 0;
            this->nextAddIndex = 0;
            this->lastSamplePosition = {PositionType::NONE, PositionType::NONE};
        }
        return true;
    };

    // 结束一个角钢
    // @return 是否成功
    bool EndTask() {
#ifdef DEBUG
        std::cout << "EndTask\n";
#endif
        {
            std::unique_lock<std::mutex> lock(this->workMutex);
            if(this->state == DetectSectionState::WORKING){
                // 如果是AVAILABLE状态，那么说明已经找到了
                this->state = DetectSectionState::WAITING;
                // 把最后部分加入队列
                for(int i = this->lastPushIndex + 1; i < this->nextAddIndex; ++i){
                    this->workQueue.push(i);
                }
            }
        }
        this->workCond.notify_one();
        return true;
    };

    // 添加一帧图像
    // @param imageForward: 顺向图像
    // @param imageBackward: 逆向图像
    // @return 是否成功
    bool AddImage(const cv::Mat& imageForward, const cv::Mat& imageBackward) {
        if(this->state != DetectSectionState::WORKING){
#ifdef DEBUG
        Log_Warning("DetectSection is not in WORKING state");
#endif
            return false;
        }
        if(this->nextAddIndex >= DETECT_SECTION_MAX_VECTOR_SIZE){
            Log_Error("DetectSection workVector is full");
            throw std::runtime_error("Error: DetectSection workVector is full");
            // return false;
        }
        {
            std::lock_guard<std::mutex> lock(this->workMutex);
            this->workVector[this->nextAddIndex] = std::move(std::make_pair(std::move(DetectSectionWork<T>(imageForward)), std::move(DetectSectionWork<T>(imageBackward))));
            if(this->nextAddIndex % this->sampleInterval == 0){
                this->workQueue.push(nextAddIndex);
                this->lastPushIndex = this->nextAddIndex;
            }
            this->nextAddIndex++;
        }
        this->workCond.notify_one();
        return true;
    };
};