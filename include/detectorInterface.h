#pragma once
#include <string>
#include <opencv2/core.hpp>

// 识别结果在图像中的位置
enum class PositionType{
    NONE,
    FINE,
    LEFT,
    RIGHT
};

// 一帧识别结果接口类
class OutputFrameinterface {
public:
    // 识别结果在图像中的位置
    PositionType position = PositionType::NONE;

    // 识别结果
    std::string result = "";

    // 是否需要翻转
    bool reverse = false;

    // 结果状态, bit0: 已获取原始结果; bit1: 已获取位置; bit2: 已获取识别结果
    int state = 0;

    virtual ~OutputFrameinterface(){};

    // 更新位置
    virtual bool UpdatePosition(PositionType position) = 0;

    // 更新识别结果
    virtual bool UpdateResult(const std::string& result) = 0;
};

// 识别器接口
// T为OutputFrameinterface的实现
template <typename T>
class DetectorInterface {
protected:
    // 可用标记
    int available;

public:
    // 析构函数
    virtual ~DetectorInterface(){};

    // 检测一帧图像
    // @param frame: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    virtual bool Detect(const cv::Mat& frame, T& output) = 0;

    // 检测结果位置
    // @param frame: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    virtual bool DetectPosition(const cv::Mat& frame, T& output) = 0;

    // 检测结果内容
    // @param frame: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    virtual bool DetectString(const cv::Mat& frame, T& output) = 0;

    // 检查是否可用
    // @return 是否可用
    virtual bool CheckAvailable() const = 0;

    // 处理output, 获取位置
    // @param output: 输出结果
    // @param width: 图像宽度
    // @return 是否成功
    virtual bool _TestPosition(T& output, int width) = 0;

    // 处理output, 获取识别结果
    // @param output: 输出结果
    // @param image: 图像
    // @return 是否成功
    virtual bool _ExtractString(T& output, const cv::Mat& image) = 0;

    // 绘制识别结果
    // @param frame: 输入图像
    // @param output: 输出结果
    virtual void DrawResult(cv::Mat& frame, const T& output) = 0;
};