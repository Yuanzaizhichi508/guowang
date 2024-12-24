#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

// Paddle文本识别器
class PaddleTextRecognizer
{
public:
	// 构造函数
	PaddleTextRecognizer();
	// 析构函数
	~PaddleTextRecognizer();

	// 初始化
	// @param modelPath: 模型路径
	// @param dictPath: 字典路径
	// @param isCuda: 是否使用CUDA
	// @return 是否成功
	bool init(
		const std::string& modelPath,
		const std::string& dictPath,
		bool isCuda = false
	);
	// 预测
	// @param cv_image: 输入图像
	// @return 预测结果
	std::string predict_text(cv::Mat cv_image);
	
private:
	// 预处理
	// @param srcimg: 输入图像
	// @return 预处理后的图像
	cv::Mat preprocess(cv::Mat srcimg);

	// 归一化
	// @param img: 输入图像
	void normalize_(cv::Mat img);

	// 输入宽度
	const int inpWidth = 320;
	// 输入高度
	const int inpHeight = 48;
	
	// 输入图像
	std::vector<float> input_image_;
	// 字典
	std::vector<std::string> alphabet;
	// 字典长度
	int names_len;
	// 预测结果
	std::vector<int> preb_label;

	// ONNX运行时环境
	Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "Paddle Text Recognizer");
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