#pragma once

#include "detectorInterface.h"
#include "timeUtil.hpp"
#include "yoloOnnx.h"
#include "yoloUtils.h"
#include "jsoncpp.h"

// YOLO输出帧
class YoloOutputFrame : public OutputFrameinterface {
public:
    // 输出
    std::vector<OutputParams> output;
    // 平均字宽
    float mean_width;
    // 平均字高
    float mean_height;

    // 构造函数
    YoloOutputFrame() {};
    // 构造函数
    // @param output: 输出
    YoloOutputFrame(const std::vector<OutputParams>& output): output(output) {};

    // 更新output
    bool UpdateOutput() {
        std::sort(this->output.begin(), this->output.end(), [](const OutputParams& a, const OutputParams& b){
            return a.box.x < b.box.x;
        });
        this->mean_width = std::accumulate(output.begin(), output.end(), 0.0, [](float sum, OutputParams& a){
            return sum + a.box.width;
        }) / output.size();
        this->mean_height = std::accumulate(output.begin(), output.end(), 0.0, [](float sum, OutputParams& a){
            return sum + a.box.height;
        }) / output.size();
        state |= 1 << 0;
        return true;
    };

    // 更新位置
    // @param position: 位置
    bool UpdatePosition(PositionType position) {
        this->position = position;
        state |= 1 << 1;
        return true;
    };

    // 更新识别结果
    // @param result: 识别结果
    bool UpdateResult(const std::string& result) {
        this->result = result;
        state |= 1 << 2;
        return true;
    };
};

// YOLO模块
class YoloOnnxDetector : public DetectorInterface<YoloOutputFrame> {
private:
    // 模型
    YoloOnnx yolo;

    // 类别
    std::vector<std::string> classes;
    // 类别反转映射
    std::vector<int> reverse_map;

    // 水平边距阈值(相对幅面宽度)
    float horizontal_margin;
    // 最小簇规模
    int min_samples;
    // 置信度阈值
    float conf;
    // NMS阈值
    float nms_threshold;
    // 宽度阈值(相对字符平均高度比例)
    float width_threshold;
    // 高度阈值(相对字符平均宽度比例)
    float height_threshold;
    // 字符串反转阈值
    float rev_threshold;

public:
    // 构造函数
    // @param horizontal_margin: 水平边距阈值
    // @param min_samples: 最小簇规模
    // @param conf: 置信度阈值
    // @param nms_threshold: NMS阈值
    // @param width_threshold: 宽度阈值(比例)
    // @param height_threshold: 高度阈值(比例)
    // @param rev_threshold: 字符串反转阈值
    YoloOnnxDetector(
        float horizontal_margin,
        int min_samples,
        float conf,
        float nms_threshold,
        float width_threshold,
        float height_threshold,
        float rev_threshold
    ) :
        horizontal_margin(horizontal_margin),
        min_samples(min_samples),
        conf(conf),
        nms_threshold(nms_threshold),
        width_threshold(width_threshold),
        height_threshold(height_threshold),
        rev_threshold(rev_threshold)
    {
        this->yolo.SetClassThreshold(conf);
        this->yolo.SetNmsThreshold(nms_threshold);
    };

    // 析构函数
    ~YoloOnnxDetector() override {};

    // 加载模型
    // @param modelPath: 模型路径
    // @param isCuda: 是否使用CUDA
    // @param cudaID: CUDA设备ID
    // @param warmUp: 是否预热
    // @return 是否成功
    bool LoadModel(const std::string& modelPath, bool isCuda = false, int cudaID = 0, bool warmUp = true) {
        bool res = this->yolo.ReadModel(modelPath, isCuda, cudaID, warmUp);
        if(res) {
            this->available |= 1 << 0;
        }
        else {
            Log_Error("Failed to load model! Please check the model " + modelPath);
        }
        return res;
    };

    // 加载类别
    // @param classesPath: 类别路径
    // @return 是否成功
    bool LoadClasses(const std::string& classesPath) {
        if(0 != _access(classesPath.c_str(), 0)){
            Log_Error("Classes path does not exist,  please check " + classesPath);
            return false;
        }
        std::ifstream file(classesPath);
        std::string line;
        while(std::getline(file, line)){
            classes.push_back(line);
        }
        this->available |= 1 << 1;
        return true;
    };

    // 加载反转映射
    // @param reverseMapPath: 反转映射路径
    // @return 是否成功
    bool LoadReverseMap(const std::string& reverseMapPath) {
        if(0 != _access(reverseMapPath.c_str(), 0)) {
            Log_Error("Reverse map path does not exist,  please check " + reverseMapPath);
            return false;
        }
        if((this->available & (1 << 1)) == 0) {
            Log_Error("Classes not loaded!");
            return false;
        }
        std::unordered_map<std::string, int> classesMap;
        for(int i = 0; i < classes.size(); i++){
            classesMap[classes[i]] = i;
        }
        reverse_map.resize(classes.size(), -1);
        Json::Value root;
        Json::Reader reader;
        std::ifstream file(reverseMapPath);
        if(!reader.parse(file, root)){
            Log_Error("[detector] LoadReverseMap(): Failed to parse reverse map! Please check " + reverseMapPath);
            return false;
        }
        // 加载reverse map
        for(auto& item : root.getMemberNames()){
            std::string key = item;
            std::string value = root[key].asString();
            if(
                classesMap.find(key) != classesMap.end() &&
                classesMap.find(value) != classesMap.end()
            ){
                reverse_map[classesMap[key]] = classesMap[value];
                reverse_map[classesMap[value]] = classesMap[key];
            }          
        }
        for(int i = 0; i < classes.size(); i++){
            if(reverse_map[i] == -1){
                reverse_map[i] = i;
            }
        }
        this->available |= 1 << 2;
        return true;
    };

    // 检查是否可用
    // @return 是否可用
    bool CheckAvailable() const override {
        if((this->available == (1 << 3)) - 1){
            return true;
        }
        if((this->available & (1 << 0)) == 0){
            Log_Warning("Model not loaded!");
        }
        if((this->available & (1 << 1)) == 0){
            Log_Warning("Classes not loaded!");
        }
        if((this->available & (1 << 2)) == 0){
            Log_Warning("Reverse map not loaded!");
        }
        return false;
    }

    // 检测
    // @param image: 待检测图像
    // @param outputFrame: 输出结果
    // @return 是否成功
    bool Detect(const cv::Mat& image, YoloOutputFrame& outputFrame) override {
        if(!CheckAvailable()){
            return false;
        }
        cv::Mat inputImg = image;
        // 检查通道数
        if(inputImg.channels() != 3){
            if(inputImg.channels() == 1){
                Log_Info("Convert to 3 channels!");
                // 灰度图转换为3通道
                cv::cvtColor(inputImg, inputImg, cv::COLOR_GRAY2BGR);
            }
            else{
                Log_Error("Image channels not supported!");
                return false;
            }
        }
        bool ret = yolo.OnnxDetect(inputImg, outputFrame.output);
        if(!ret){
            return false;
        }
        outputFrame.UpdateOutput();
        if((outputFrame.state & (1 << 0)) == 0){
            return false;
        }
        return true;
    };

    // 检测位置
    // @param image: 待检测图像
    // @param outputFrame: 输出结果
    // @return 是否成功
    bool DetectPosition(const cv::Mat& image, YoloOutputFrame& outputFrame) override {
        if(!CheckAvailable()){
            return false;
        }
        if(Detect(image, outputFrame)){
            return _TestPosition(outputFrame, image.cols);
        }
        return false;
    }

    // 检测字符串
    // @param image: 待检测图像
    // @param outputFrame: 输出结果
    // @return 是否成功
    bool DetectString(const cv::Mat& image, YoloOutputFrame& outputFrame) override {
        if(!CheckAvailable()){
            return false;
        }
        if(Detect(image, outputFrame)){
            return _ExtractString(outputFrame, image);
        }
        return false;
    }

    // 处理output, 获取位置
    // @param outputFrame: 输出结果
    // @param width: 图像宽度
    // @return 是否成功
    bool _TestPosition(YoloOutputFrame& outputFrame, int width) override {
        // 空的
        if(outputFrame.output.size() < this->min_samples){
            outputFrame.UpdatePosition(PositionType::NONE);
            return true;
        }
        if(outputFrame.output[0].box.x < this->horizontal_margin * width){
            outputFrame.UpdatePosition(PositionType::LEFT);
            return true;
        }
        if(width - (outputFrame.output.back().box.x + outputFrame.output.back().box.width) < this->horizontal_margin * width){
            outputFrame.UpdatePosition(PositionType::RIGHT);
            return true;
        }
        outputFrame.UpdatePosition(PositionType::FINE);
        return true;
    };

    // 处理output, 获取识别结果
    // @param outputFrame: 输出结果
    // @param image: 图像
    // @return 是否成功
    bool _ExtractString(YoloOutputFrame& outputFrame, const cv::Mat& image) override {
#ifdef DEBUG
        std::ostringstream oss;
        auto print_middle_result = [&](std::vector<int>& raw_result){
            oss << "|\t";
            for(int i : raw_result){
                oss << this->classes[outputFrame.output[i].id];
            }
            oss << std::endl;
        };
        
        oss << "====================\n| output: \n|\t";
        for(auto& out : outputFrame.output){
            oss << this->classes[out.id];
        }
        oss << "\n====================" << std::endl;
        std::cout << oss.str();
#endif

        if((outputFrame.state & (1 << 0)) == 0 || (this->available & (1 << 2)) == 0){
            return false;
        }
        if(outputFrame.output.size() == 0){
            outputFrame.UpdateResult(std::move(std::string()));
            return true;
        }
        // 圆边界
        // float radiusSquare = this->width_threshold * this->width_threshold * outputFrame.mean_width * outputFrame.mean_width;
        // std::function<bool(OutputParams&, OutputParams&)> isClose = [radiusSquare](OutputParams& a, OutputParams& b){
        //     return (a.box.x - b.box.x) * (a.box.x - b.box.x) + (a.box.y - b.box.y) * (a.box.y - b.box.y) < radiusSquare;
        // };
        // 矩形边界
        std::function<bool(OutputParams&, OutputParams&)> isClose = [&](OutputParams& a, OutputParams& b){
            return std::abs(a.box.x - b.box.x) < this->width_threshold * outputFrame.mean_width &&
                std::abs(a.box.y - b.box.y) < this->width_threshold * outputFrame.mean_width;
        };
        std::string result;
        // 簇
        std::vector<std::pair<float, std::vector<int>>> clusters;
        // 合并后的簇
        std::vector<std::pair<float, std::vector<int>>> merged_clusters;
        // 粗提取的识别序列
        std::vector<int> raw_result;
        // 总计数
        int count = 0;
        // 反向计数
        int rev_count = 0;

        // 首先把output分成若干个簇
        for(int i = 0; i < outputFrame.output.size(); i++){
            OutputParams& param = outputFrame.output[i];
            if(clusters.size() == 0){
                clusters.push_back(std::pair<float, std::vector<int>>{0, std::vector<int>{i}});
                continue;
            }
            bool found = false;
            for(auto& [mean_y, cluster] : clusters){
                if(isClose(param, outputFrame.output[cluster.back()])){
                    cluster.push_back(i);
                    found = true;
                    break;
                }
            }
            if(!found){
                clusters.push_back(std::pair<float, std::vector<int>>{0, std::vector<int>{i}});
            }
        }
        // 计算簇的平均y坐标
        for(auto& [mean_y, cluster] : clusters){
            mean_y = std::accumulate(cluster.begin(), cluster.end(), 0.0, [&](float sum, int i){
                return sum + outputFrame.output[i].box.y;
            }) / cluster.size();
        }

#ifdef DEBUG
        oss.clear();
        oss << "====================\n| clusters: " << std::endl;
        for(auto& [mean_y, cluster] : clusters){
            print_middle_result(cluster);
        }
        oss << "====================" << std::endl;
        std::cout << oss.str();
#endif

        // 由于用于流水线，此处只考虑识别一个钢印的情况，故仅合并出一个块
        // 簇合并
        for(int i = 0; i < clusters.size(); i++){
            std::pair<float, std::vector<int>>& cluster = clusters[i];
            if(merged_clusters.size() == 0){
                merged_clusters.push_back(cluster);
                continue;
            }
            bool merged = false;
            for(auto& [mean_y, merged_cluster] : merged_clusters){
                if(std::abs(cluster.first - mean_y) < this->height_threshold * outputFrame.mean_height){  
                    // 更新平均y坐标
                    mean_y = (mean_y * merged_cluster.size() + cluster.first * cluster.second.size()) / (merged_cluster.size() + cluster.second.size());
                    // 合并行
                    merged_cluster.insert(merged_cluster.end(), cluster.second.begin(), cluster.second.end());
                    merged = true;
                    break;
                }
            }
            if(!merged){
                merged_clusters.push_back(cluster);
            }
        }

        // 簇排序
        std::sort(merged_clusters.begin(), merged_clusters.end(), [](const std::pair<float, std::vector<int>>& a, const std::pair<float, std::vector<int>>& b){
            return a.first < b.first;
        });

#ifdef DEBUG
        oss.clear();
        oss << "====================\n| merged clusters: " << std::endl;
        for(auto& [mean_y, cluster] : merged_clusters){
            print_middle_result(cluster);
        }
        oss << "====================" << std::endl;
        std::cout << oss.str();
#endif
        
        // 提取字符串
        // 先按顺序拼起来
        for(int cluster_idx = 0; cluster_idx < merged_clusters.size(); cluster_idx++){
            auto& [mean_y, cluster] = merged_clusters[cluster_idx];
            // 这个时候每行内元素数还比较少的多少有问题
            if(cluster.size() < this->min_samples){
                continue;
            }
            // 按照x坐标排序
            std::sort(cluster.begin(), cluster.end(), [&](int a, int b){
                return outputFrame.output[a].box.x < outputFrame.output[b].box.x;
            });
            for(int i = 0; i < cluster.size(); i++){
                int idx = cluster[i];
                //检查与之前一个字符的x坐标差
                if(i > 0){
                    if(outputFrame.output[idx].box.x - outputFrame.output[cluster[i - 1]].box.x > 1.5 * outputFrame.mean_width){
                        raw_result.push_back(-1);
                    }
                }
                if(outputFrame.output[idx].id >= 66){
                    rev_count++;
                }
                raw_result.push_back(outputFrame.output[idx].id);
                count++;
            }
            if(cluster_idx < merged_clusters.size() - 1){
                raw_result.push_back(-1);
            }
        }

        if(rev_count > count * this->rev_threshold){
            outputFrame.reverse = true;
            std::reverse(raw_result.begin(), raw_result.end());
            for(int i = 0; i < raw_result.size(); i++){
                if(raw_result[i] < 0){
                    continue;
                }
                raw_result[i] = reverse_map[raw_result[i]];
            }
        }
        for(int i : raw_result){
            if(i < 0){
                result += " ";
            }
            else if(i >= 66){
                result += classes[this->reverse_map[i]];
            }
            else{
                result += classes[i];
            }
        }

        outputFrame.UpdateResult(result);

        return true;
    };

    // 绘制结果
    // @param image: 待绘制图像
    // @param outputFrame: 输出结果
    void DrawResult(cv::Mat& image, const YoloOutputFrame& outputFrame) override {
        // 检查是否已经完成检测，以及是否存在classes
        if((outputFrame.state & (1 << 0)) == 0 || (this->available & (1 << 1)) == 0){
            return;
        }
        std::vector<OutputParams> output(outputFrame.output);
        if(outputFrame.state & (1 << 2)){
            // 已经检测了字符串，可以获取正反向
            if(
                outputFrame.reverse &&
                this->available & (1 << 2)
            ){
                // 首先反转图片
                cv::flip(image, image, -1);
                // 然后反转结果
                for(auto& out : output){
                    out.box.x = image.cols - out.box.x - out.box.width;
                    out.box.y = image.rows - out.box.y - out.box.height;
                    out.id = reverse_map[out.id];
                }
            }
        }
        for(auto& out : output){
            cv::rectangle(image, out.box, cv::Scalar(0, 255, 0), 1);
            cv::putText(image, this->classes[out.id], cv::Point(out.box.x, out.box.y - 1), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        }
    };
};
