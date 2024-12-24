#pragma once

#include <iostream>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <io.h>

#define ORT_OLD_VISON 17

// YOLO输出参数
struct OutputParams {
	int id;             // 类型id
	float confidence;   // 置信度
	cv::Rect box;       // 框
	cv::Mat boxMask;    // 掩码
};

// 检查模型路径
bool CheckModelPath(std::string modelPath);

// 检查参数
bool CheckParams(int netHeight, int netWidth, const int* netStride, int strideSize);

// 保持比例缩放并添加填充
// @param image: 输入图像
// @param outImage: 输出图像
// @param params: 形状参数[ratio_x,ratio_y,dw,dh]
// @param newShape: 新形状
// @param autoShape: 是否自动调整形状
// @param scaleFill: 是否填充
// @param scaleUp: 是否放大
// @param stride: 步长
// @param color: 颜色
void LetterBox(
	const cv::Mat& image, 
	cv::Mat& outImage,
	cv::Vec4d& params,
	const cv::Size& newShape = cv::Size(640, 640),
	bool autoShape = false,
	bool scaleFill = false,
	bool scaleUp = true,
	int stride = 32,
	const cv::Scalar& color = cv::Scalar(0, 0, 0)
);
