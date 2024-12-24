#pragma once

#include <fstream>
#include <iostream>
#include "jsoncpp.h"
#include "timeUtil.hpp"

// config文件位置
#define CONFIG_PATH                             "data/config.json"

// 默认参数
#define DEFAULT_DEVICE                          "cpu"
#define DEFAULT_HORIZONTAL_MARGIN               0.1f
#define DEFAULT_HEIGHT_THRESHOLD                1.5f
#define DEFAULT_REV_THRESHOLD                   0.3f

#define DEFAULT_YOLO_CLASSES_LIST_PATH          "data/yolo/classes.txt"
#define DEFAULT_YOLO_REVERSE_MAP_PATH           "data/yolo/rev_map.json"
#define DEFAULT_YOLO_MODEL_PATH                 "data/yolo/model.onnx"
#define DEFAULT_YOLO_CONF                       0.4f
#define DEFAULT_YOLO_IOU                        0.5f
#define DEFAULT_YOLO_NMS_THRESHOLD              0.3f
#define DEFAULT_YOLO_WIDTH_THRESHOLD            1.8f
#define DEFAULT_YOLO_MIN_SAMPLES                1
#define DEFAULT_YOLO_CUT_OFF                    0.7f

#define DEFAULT_PADDLE_DET_MODEL_PATH           "data/paddle/det.onnx"
#define DEFAULT_PADDLE_DET_BINARY_THRESHOLD     0.3f
#define DEFAULT_PADDLE_DET_POLYGON_THRESHOLD    0.5f
#define DEFAULT_PADDLE_DET_UNCLIP_RATIO         1.6f
#define DEFAULT_PADDLE_DET_MAX_CANDIDATES       1000
#define DEFAULT_PADDLE_CLS_MODEL_PATH           "data/paddle/cls.onnx"
#define DEFAULT_PADDLE_REC_MODEL_PATH           "data/paddle/rec.onnx"
#define DEFAULT_PADDLE_REC_DICT_PATH            "data/paddle/dict.txt"

#define DEFAULT_CONFUSION_MATRIX_PATH           "data/confusion_matrix_normalized.csv"
#define DEFAULT_DISTANCE_THRESHOLD              3.0f
#define DEFAULT_DISTANCE_DIFF_THRESHOLD         0.2f

class Config
{
private:
    // 模型通用参数
    std::string     device                      = DEFAULT_DEVICE;                       // 设备
    float           horizontalMargin            = DEFAULT_HORIZONTAL_MARGIN;            // 水平边距
    float           heightThreshold             = DEFAULT_HEIGHT_THRESHOLD;             // 高度阈值
    float           revThreshold                = DEFAULT_REV_THRESHOLD;                // 字符串反转阈值

    // 模型参数(YOLO)
    std::string     classesListPath             = DEFAULT_YOLO_CLASSES_LIST_PATH;       // 类别列表路径
    std::string     reverseMapPath              = DEFAULT_YOLO_REVERSE_MAP_PATH;        // 反转映射路径
    std::string     modelPath                   = DEFAULT_YOLO_MODEL_PATH;              // 模型路径
    float           conf                        = DEFAULT_YOLO_CONF;                    // 置信度
    float           iou                         = DEFAULT_YOLO_IOU;                     // IOU
    float           nms_threshold               = DEFAULT_YOLO_NMS_THRESHOLD;           // NMS阈值
    float           width_threshold             = DEFAULT_YOLO_WIDTH_THRESHOLD;         // 宽度阈值
    int             min_samples                 = DEFAULT_YOLO_MIN_SAMPLES;             // 最小簇规模

    // 模型参数(Paddle)
    std::string     paddleDetModelPath          = DEFAULT_PADDLE_DET_MODEL_PATH;        // 文本检测模型路径
    float           paddleDetBinaryThreshold    = DEFAULT_PADDLE_DET_BINARY_THRESHOLD;  // 文本检测二值化阈值
    float           paddleDetPolygonThreshold   = DEFAULT_PADDLE_DET_POLYGON_THRESHOLD; // 文本检测多边形阈值
    float           paddleDetUnclipRatio        = DEFAULT_PADDLE_DET_UNCLIP_RATIO;      // 文本检测unclip比例
    int             paddleDetMaxCandidates      = DEFAULT_PADDLE_DET_MAX_CANDIDATES;    // 文本检测最大候选数
    std::string     paddleClsModelPath          = DEFAULT_PADDLE_CLS_MODEL_PATH;        // 文本分类模型路径
    std::string     paddleRecModelPath          = DEFAULT_PADDLE_REC_MODEL_PATH;        // 文本识别模型路径
    std::string     paddleRecDictPath           = DEFAULT_PADDLE_REC_DICT_PATH;         // 文本识别字典路径

    // 匹配参数
    std::string     confusionMatrixPath         = DEFAULT_CONFUSION_MATRIX_PATH;        // 混淆矩阵路径
    float           distanceThreshold           = DEFAULT_DISTANCE_THRESHOLD;           // 距离阈值
    float           distanceDiffThreshold       = DEFAULT_DISTANCE_DIFF_THRESHOLD;      // 距离差值阈值

private:
    // 序列化并写入配置文件
    // 返回值: 0 - 成功序列化并保存； 1 - 输出失败
    int Serilize(const std::string& outputFilePath){
        // 输出文件流
        std::ofstream outputFile(outputFilePath);
        if(!outputFile.is_open()){
            return 1;
        }

        // Json节点
        Json::Value root;

        // 节点挂载内容
        root["ModelParams"]["Device"] = this->device;
        root["ModelParams"]["HorizontalMargin"] = this->horizontalMargin;
        root["ModelParams"]["HeightThreshold"] = this->heightThreshold;
        root["ModelParams"]["RevThreshold"] = this->revThreshold;

        root["ModelParams"]["YOLO"]["ClassesListPath"] = this->classesListPath;
        root["ModelParams"]["YOLO"]["ReverseMapPath"] = this->reverseMapPath;
        root["ModelParams"]["YOLO"]["ModelPath"] = this->modelPath;
        root["ModelParams"]["YOLO"]["Conf"] = this->conf;
        root["ModelParams"]["YOLO"]["Iou"] = this->iou;
        root["ModelParams"]["YOLO"]["NmsThreshold"] = this->nms_threshold;
        root["ModelParams"]["YOLO"]["WidthThreshold"] = this->width_threshold;
        root["ModelParams"]["YOLO"]["MinSamples"] = this->min_samples;
        
        root["ModelParams"]["Paddle"]["DetModelPath"] = this->paddleDetModelPath;
        root["ModelParams"]["Paddle"]["DetBinaryThreshold"] = this->paddleDetBinaryThreshold;
        root["ModelParams"]["Paddle"]["DetPolygonThreshold"] = this->paddleDetPolygonThreshold;
        root["ModelParams"]["Paddle"]["DetUnclipRatio"] = this->paddleDetUnclipRatio;
        root["ModelParams"]["Paddle"]["DetMaxCandidates"] = this->paddleDetMaxCandidates;
        root["ModelParams"]["Paddle"]["ClsModelPath"] = this->paddleClsModelPath;
        root["ModelParams"]["Paddle"]["RecModelPath"] = this->paddleRecModelPath;
        root["ModelParams"]["Paddle"]["RecDictPath"] = this->paddleRecDictPath;

        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "\t";

        std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
        writer->write(root, &outputFile);

        outputFile.close();
        return 0;
    };

    // 从配置文件读取配置
    // 返回值： 0 - 成功读取配置文件； 1 - 打开配置文件失败； 2 - 解析配置文件失败； 3 - 配置文件错误
    int Deserilize(const std::string& inputFilePath){
        // 输入文件流
        std::ifstream inputFile(inputFilePath);
        if(!inputFile.is_open()){
            return 1;
        }

        // Json解析器
        Json::Reader reader;
        // Json节点
        Json::Value root;

        if(!reader.parse(inputFile, root)){
            inputFile.close();
            return 2;
        }

        // 检查Json结构
        if( root.isMember("ModelParams") &&
                root["ModelParams"].isMember("Device") && root["ModelParams"]["Device"].type() == Json::stringValue &&
                root["ModelParams"].isMember("HorizontalMargin") && root["ModelParams"]["HorizontalMargin"].type() == Json::realValue &&
                root["ModelParams"].isMember("HeightThreshold") && root["ModelParams"]["HeightThreshold"].type() == Json::realValue &&
                root["ModelParams"].isMember("RevThreshold") && root["ModelParams"]["RevThreshold"].type() == Json::realValue &&
                root["ModelParams"].isMember("YOLO") && 
                    root["ModelParams"]["YOLO"].isMember("ClassesListPath") && root["ModelParams"]["YOLO"]["ClassesListPath"].type() == Json::stringValue &&
                    root["ModelParams"]["YOLO"].isMember("ReverseMapPath") && root["ModelParams"]["YOLO"]["ReverseMapPath"].type() == Json::stringValue &&
                    root["ModelParams"]["YOLO"].isMember("ModelPath") && root["ModelParams"]["YOLO"]["ModelPath"].type() == Json::stringValue &&
                    root["ModelParams"]["YOLO"].isMember("Conf") && root["ModelParams"]["YOLO"]["Conf"].type() == Json::realValue &&
                    root["ModelParams"]["YOLO"].isMember("Iou") && root["ModelParams"]["YOLO"]["Iou"].type() == Json::realValue &&
                    root["ModelParams"]["YOLO"].isMember("NmsThreshold") && root["ModelParams"]["YOLO"]["NmsThreshold"].type() == Json::realValue &&
                    root["ModelParams"]["YOLO"].isMember("WidthThreshold") && root["ModelParams"]["YOLO"]["WidthThreshold"].type() == Json::realValue &&
                    root["ModelParams"]["YOLO"].isMember("MinSamples") && root["ModelParams"]["YOLO"]["MinSamples"].type() == Json::intValue &&
                root["ModelParams"].isMember("Paddle") &&
                    root["ModelParams"]["Paddle"].isMember("DetModelPath") && root["ModelParams"]["Paddle"]["DetModelPath"].type() == Json::stringValue &&
                    root["ModelParams"]["Paddle"].isMember("DetBinaryThreshold") && root["ModelParams"]["Paddle"]["DetBinaryThreshold"].type() == Json::realValue &&
                    root["ModelParams"]["Paddle"].isMember("DetPolygonThreshold") && root["ModelParams"]["Paddle"]["DetPolygonThreshold"].type() == Json::realValue &&
                    root["ModelParams"]["Paddle"].isMember("DetUnclipRatio") && root["ModelParams"]["Paddle"]["DetUnclipRatio"].type() == Json::realValue &&
                    root["ModelParams"]["Paddle"].isMember("DetMaxCandidates") && root["ModelParams"]["Paddle"]["DetMaxCandidates"].type() == Json::intValue &&
                    root["ModelParams"]["Paddle"].isMember("ClsModelPath") && root["ModelParams"]["Paddle"]["ClsModelPath"].type() == Json::stringValue &&
                    root["ModelParams"]["Paddle"].isMember("RecModelPath") && root["ModelParams"]["Paddle"]["RecModelPath"].type() == Json::stringValue &&
                    root["ModelParams"]["Paddle"].isMember("RecDictPath") && root["ModelParams"]["Paddle"]["RecDictPath"].type() == Json::stringValue &&
            root.isMember("MatcherParams") &&
                root["MatcherParams"].isMember("ConfusionMatrixPath") &&
                root["MatcherParams"].isMember("DistanceThreshold") && root["MatcherParams"]["DistanceThreshold"].type() == Json::realValue &&
                root["MatcherParams"].isMember("DistanceDiffThreshold") && root["MatcherParams"]["DistanceDiffThreshold"].type() == Json::realValue
        )
        {
            this->device = root["ModelParams"]["Device"].asString();
            this->horizontalMargin = root["ModelParams"]["HorizontalMargin"].asFloat();
            this->heightThreshold = root["ModelParams"]["HeightThreshold"].asFloat();
            this->revThreshold = root["ModelParams"]["RevThreshold"].asFloat();

            this->classesListPath = root["ModelParams"]["YOLO"]["ClassesListPath"].asString();
            this->reverseMapPath = root["ModelParams"]["YOLO"]["ReverseMapPath"].asString();
            this->modelPath = root["ModelParams"]["YOLO"]["ModelPath"].asString();
            this->conf = root["ModelParams"]["YOLO"]["Conf"].asFloat();
            this->iou = root["ModelParams"]["YOLO"]["Iou"].asFloat();
            this->nms_threshold = root["ModelParams"]["YOLO"]["NmsThreshold"].asFloat();
            this->width_threshold = root["ModelParams"]["YOLO"]["WidthThreshold"].asFloat();
            this->min_samples = root["ModelParams"]["YOLO"]["MinSamples"].asInt();

            this->paddleDetModelPath = root["ModelParams"]["Paddle"]["DetModelPath"].asString();
            this->paddleDetBinaryThreshold = root["ModelParams"]["Paddle"]["DetBinaryThreshold"].asFloat();
            this->paddleDetPolygonThreshold = root["ModelParams"]["Paddle"]["DetPolygonThreshold"].asFloat();
            this->paddleDetUnclipRatio = root["ModelParams"]["Paddle"]["DetUnclipRatio"].asFloat();
            this->paddleDetMaxCandidates = root["ModelParams"]["Paddle"]["DetMaxCandidates"].asInt();
            this->paddleClsModelPath = root["ModelParams"]["Paddle"]["ClsModelPath"].asString();
            this->paddleRecModelPath = root["ModelParams"]["Paddle"]["RecModelPath"].asString();
            this->paddleRecDictPath = root["ModelParams"]["Paddle"]["RecDictPath"].asString();
        }
        else{
            inputFile.close();
            return 3;
        }

        inputFile.close();
        return 0;
    };

    Config(/* args */){
        if(this->Deserilize(CONFIG_PATH) == 0){
            Log_Info("读取配置文件 " + std::string(CONFIG_PATH));
            return;
        }
        else if(this->Serilize(CONFIG_PATH) == 0){
            Log_Warning("读取配置文件失败, 写入默认配置到 " + std::string(CONFIG_PATH));
            return;
        }
        else{
            Log_Error("读取配置文件失败, 写入默认配置失败");
            exit(50001);
        }
    };

public:

    static Config& GetInstance(){
        static Config cfg;
        return cfg;
    };

    static std::string GetDevice(){
        return GetInstance().device;
    };

    static float GetHorizontalMargin(){
        return GetInstance().horizontalMargin;
    };

    static float GetHeightThreshold(){
        return GetInstance().heightThreshold;
    };

    static float GetRevThreshold(){
        return GetInstance().revThreshold;
    };

    static std::string GetClassesListPath(){
        return GetInstance().classesListPath;
    };

    static std::string GetReverseMapPath(){
        return GetInstance().reverseMapPath;
    };

    static std::string GetModelPath(){
        return GetInstance().modelPath;
    };

    static float GetConf(){
        return GetInstance().conf;
    };

    static float GetIou(){
        return GetInstance().iou;
    };

    static float GetNmsThreshold(){
        return GetInstance().nms_threshold;
    };

    static float GetWidthThreshold(){
        return GetInstance().width_threshold;
    };

    static int GetMinSamples(){
        return GetInstance().min_samples;
    };

    static std::string GetPaddleDetModelPath(){
        return GetInstance().paddleDetModelPath;
    };

    static float GetPaddleDetBinaryThreshold(){
        return GetInstance().paddleDetBinaryThreshold;
    };

    static float GetPaddleDetPolygonThreshold(){
        return GetInstance().paddleDetPolygonThreshold;
    };

    static float GetPaddleDetUnclipRatio(){
        return GetInstance().paddleDetUnclipRatio;
    };

    static int GetPaddleDetMaxCandidates(){
        return GetInstance().paddleDetMaxCandidates;
    };

    static std::string GetPaddleClsModelPath(){
        return GetInstance().paddleClsModelPath;
    };

    static std::string GetPaddleRecModelPath(){
        return GetInstance().paddleRecModelPath;
    };

    static std::string GetPaddleRecDictPath(){
        return GetInstance().paddleRecDictPath;
    };

    static std::string GetConfusionMatrixPath(){
        return GetInstance().confusionMatrixPath;
    };

    static float GetDistanceThreshold(){
        return GetInstance().distanceThreshold;
    };

    static float GetDistanceDiffThreshold(){
        return GetInstance().distanceDiffThreshold;
    };
};