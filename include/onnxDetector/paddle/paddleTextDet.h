#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

// Paddle检测器
class PaddleTextDetector
{
public:
	// 构造函数
	PaddleTextDetector();
	// 析构函数
	~PaddleTextDetector();

	// 初始化
	// @param modelPath: 模型路径
	// @param binaryThreshold: 二值化阈值
	// @param polygonThreshold: 多边形阈值
	// @param unclipRatio: 扩展比例
	// @param maxCandidates: 最大候选数
	// @param isCuda: 是否使用CUDA
	bool init(
		const std::string& modelPath,
		float binaryThreshold = 0.3f,
		float polygonThreshold = 0.5f,
		float unclipRatio = 1.6f,
		int maxCandidates = 1000,
		bool isCuda = false
	);

	// 检测
	// @param srcimg: 输入图像
	// @return 检测结果，每个元素为一个多边形的四个顶点
	std::vector<std::vector<cv::Point2f>> detect(cv::Mat& srcimg);
	
	// 绘制检测结果
	// @param srcimg: 输入图像
	// @param results: 检测结果
	void draw_pred(cv::Mat& srcimg, std::vector<std::vector<cv::Point2f>> results);
	
	// 获取旋转裁剪图像
	// @param frame: 输入图像
	// @param vertices: 四个顶点
	// @return 旋转裁剪图像
	cv::Mat get_rotate_crop_image(const cv::Mat& frame, std::vector<cv::Point2f> vertices);
private:
	// 二值化阈值
	float binaryThreshold;
	// 多边形阈值
	float polygonThreshold;
	// 扩展比例
	float unclipRatio;
	// 最大候选数
	int maxCandidates;
	// 长边阈值
	const int longSideThresh = 3;
	// 短边阈值
	const int short_size = 736;
	// 归一化参数
	const float meanValues[3] = { 0.485, 0.456, 0.406 };
	const float normValues[3] = { 0.229, 0.224, 0.225 };
	// 输入图像
	std::vector<float> input_image_;
	// ONNX会话
	Ort::Session *ort_session = nullptr;
	// ONNX执行环境
	Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "Paddle Text Detector");
	// ONNX会话选项
	Ort::SessionOptions sessionOptions = Ort::SessionOptions();
	// 输入节点名称
	std::vector<char*> input_names;
	// 输出节点名称
	std::vector<char*> output_names;
	
	// 计算轮廓得分
	// @param binary: 二值图像
	// @param contour: 轮廓
	// @return 轮廓得分
	float contourScore(const cv::Mat& binary, const std::vector<cv::Point>& contour);
	
	// 扩展多边形
	// @param inPoly: 输入多边形
	// @param outPoly: 输出多边形
	void unclip(const std::vector<cv::Point2f>& inPoly, std::vector<cv::Point2f> &outPoly);
	
	// 排序四个顶点
	// @param results: 四个顶点
	// @return 排序后的四个顶点
	std::vector<std::vector<cv::Point2f> > order_points_clockwise(std::vector<std::vector<cv::Point2f>> results);
	
	// 预处理
	// @param srcimg: 输入图像
	// @return 预处理后的图像
	cv::Mat preprocess(cv::Mat srcimg);

	// 归一化
	// @param img: 输入图像
	void normalize_(cv::Mat img);
}; 
