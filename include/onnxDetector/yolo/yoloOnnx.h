#pragma once

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include "yoloUtils.h"
#include <onnxruntime_cxx_api.h>

//#include <tensorrt_provider_factory.h>  //if use OrtTensorRTProviderOptionsV2
//#include <onnxruntime_c_api.h>


class YoloOnnx {
public:
	YoloOnnx() :_OrtMemoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtDeviceAllocator, OrtMemType::OrtMemTypeCPUOutput)) {};
	~YoloOnnx() {
		if (_OrtSession != nullptr)
			delete _OrtSession;
	
	};// delete _OrtMemoryInfo;


public:
	// 读取模型
	// @param modelPath: 模型路径
	// @param isCuda: 是否使用CUDA
	// @param cudaID: CUDA设备ID
	// @param warmUp: 是否预热
	// @return 是否成功
	bool ReadModel(const std::string& modelPath, bool isCuda = false, int cudaID = 0, bool warmUp = true);

	// 检测
	// @param srcImg: 输入图像
	// @param output: 输出结果
	// @return 是否成功
	bool OnnxDetect(cv::Mat& srcImg, std::vector<OutputParams>& output);
	
	// 批量检测
	// @param srcImg: 输入图像
	// @param output: 输出结果
	// @return 是否成功
	bool OnnxBatchDetect(std::vector<cv::Mat>& srcImg, std::vector<std::vector<OutputParams>>& output);

	// 设置类型置信度阈值
	// @param classThreshold: 类型置信度阈值
	void SetClassThreshold(float classThreshold);

	// 设置NMS阈值
	// @param nmsThreshold: NMS阈值
	void SetNmsThreshold(float nmsThreshold);

private:

	// 向量求积
	// @param v: 输入向量序列
	// @return 向量求积结果
	template <typename T>
	T VectorProduct(const std::vector<T>& v)
	{
		return std::accumulate(v.begin(), v.end(), 1, std::multiplies<T>());
	};

	// 预处理
	// @param srcImgs: 输入图像
	// @param outSrcImgs: 输出图像
	// @param params: 参数
	// @return 是否成功
	int Preprocessing(const std::vector<cv::Mat>& srcImgs, std::vector<cv::Mat>& outSrcImgs, std::vector<cv::Vec4d>& params);

	// ONNX输入宽度
	const int _netWidth = 640;
	// ONNX输入高度
	const int _netHeight = 640;

	// 若为批处理，设置此项
	int _batchSize = 1;
	// 是否为动态形状输入
	bool _isDynamicShape = false;
	// 类型置信度阈值
	float _classThreshold = 0.25f;
	// NMS阈值
	float _nmsThreshold = 0.45f;


	// ONNX环境
	Ort::Env _OrtEnv = Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_ERROR, "Yolo");
	// ONNX会话选项
	Ort::SessionOptions _OrtSessionOptions = Ort::SessionOptions();
	// ONNX会话
	Ort::Session* _OrtSession = nullptr;
	// ONNX内存信息
	Ort::MemoryInfo _OrtMemoryInfo;
#if ORT_API_VERSION < ORT_OLD_VISON
	char* _inputName, * _output_name0;
#else
	// 用于存储输入输出节点名称
	std::shared_ptr<char> _inputName, _output_name0;
#endif

	// 输入节点名称
	std::vector<char*> _inputNodeNames; 
	// 输出节点名称
	std::vector<char*> _outputNodeNames;

	// 输入节点数量
	size_t _inputNodesNum = 0;
	// 输出节点数量
	size_t _outputNodesNum = 0;

	// 输入节点数据类型
	ONNXTensorElementDataType _inputNodeDataType;
	// 输出节点数据类型
	ONNXTensorElementDataType _outputNodeDataType;
	// 输入节点形状
	std::vector<int64_t> _inputTensorShape;
	// 输出节点形状
	std::vector<int64_t> _outputTensorShape;

public:
	// 类别名
	std::vector<std::string> _className;
};