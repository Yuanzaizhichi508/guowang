#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

// Paddle方向分类器
class PaddleTextClassifier
{
public:
	// 构造函数
	PaddleTextClassifier();
	// 析构函数
	~PaddleTextClassifier();

	// 初始化
	// @param modelPath: 模型路径
	// @param isCuda: 是否使用CUDA
	bool init(
		const std::string& modelPath,
		bool isCuda = false
	);
	// 预测
	// @param cv_image: 输入图像
	// @return 预测结果，0为正向，1为反向
	int predict(cv::Mat cv_image);

private:
	// 分类标签
	const int label_list[2] = { 0, 180 };

	// 预处理
	// @param srcimg: 输入图像
	// @return 预处理后的图像
	cv::Mat preprocess(cv::Mat srcimg);

	// 归一化
	// @param img: 输入图像
	void normalize_(cv::Mat img);

	// 输入宽度
	const int inpWidth = 192;
	// 输入高度
	const int inpHeight = 48;
	// 输出数量
	int num_out;
	// 输入图像
	std::vector<float> input_image_;

	// ONNX运行时环境
	Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "Paddle Text Classifier");
	// ONNX会话
	Ort::Session *ort_session = nullptr;
	// ONNX会话选项
	Ort::SessionOptions sessionOptions = Ort::SessionOptions();

	// 输入节点名称
	std::vector<char*> input_names;
	// 输出节点名称
	std::vector<char*> output_names;
	// 输入节点形状
	std::vector<std::vector<int64_t>> input_node_dims; // >=1 outputs
	// 输出节点形状
	std::vector<std::vector<int64_t>> output_node_dims; // >=1 outputs
};