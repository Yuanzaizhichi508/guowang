#include "paddleTextCls.h"
#include <filesystem>

using namespace Ort;

PaddleTextClassifier::PaddleTextClassifier(){}

PaddleTextClassifier::~PaddleTextClassifier()
{
	if (ort_session != nullptr)
	{
		delete ort_session;
	}
	for (int i = 0; i < input_names.size(); i++)
	{
		delete[] input_names[i];
	}
	for (int i = 0; i < output_names.size(); i++)
	{
		delete[] output_names[i];
	}
}

bool PaddleTextClassifier::init(const std::string& modelPath, bool isCuda)
{
	// 检查模型文件是否存在，是否为onnx文件
	if (
		!std::filesystem::exists(modelPath) ||
		modelPath.substr(modelPath.find_last_of(".") + 1) != "onnx"
	) {
		return false;
	}

	try {
		// 查找可用执行环境
		std::vector<std::string> available_providers = GetAvailableProviders();
		auto cuda_available = std::find(available_providers.begin(), available_providers.end(), "CUDAExecutionProvider");

		// CUDA不可用
		if (isCuda && (cuda_available == available_providers.end()))
		{
			std::cout << "Your ORT build without GPU. Change to CPU." << std::endl;
			std::cout << "************* Infer model on CPU! *************" << std::endl;
		}
		// CUDA可用
		else if (isCuda && (cuda_available != available_providers.end()))
		{
			std::cout << "************* Infer model on GPU! *************" << std::endl;
			OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);
		}
		else
		{
			std::cout << "************* Infer model on CPU! *************" << std::endl;
		}

		sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

		std::wstring widestr = std::wstring(modelPath.begin(), modelPath.end());
		ort_session = new Session(env, widestr.c_str(), sessionOptions);
		size_t numInputNodes = ort_session->GetInputCount();
		size_t numOutputNodes = ort_session->GetOutputCount();
		AllocatorWithDefaultOptions allocator;
		for (int i = 0; i < numInputNodes; i++)
		{
			char* copied_input_name = new char[strlen(ort_session->GetInputNameAllocated(i, allocator).get()) + 1];
			strcpy(copied_input_name, ort_session->GetInputNameAllocated(i, allocator).get());
			input_names.push_back(copied_input_name);
			TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
			auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
			auto input_dims = input_tensor_info.GetShape();
			input_node_dims.push_back(input_dims);
		}
		for (int i = 0; i < numOutputNodes; i++)
		{
			char* copied_output_name = new char[strlen(ort_session->GetOutputNameAllocated(i, allocator).get()) + 1];
			strcpy(copied_output_name, ort_session->GetOutputNameAllocated(i, allocator).get());
			output_names.push_back(copied_output_name);
			TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
			auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
			auto output_dims = output_tensor_info.GetShape();
			output_node_dims.push_back(output_dims);
		}
	}
	catch (const std::exception& e) {
		return false;
	}
	return true;
}

cv::Mat PaddleTextClassifier::preprocess(cv::Mat srcimg)
{
	cv::Mat dstimg;
	int h = srcimg.rows;
	int w = srcimg.cols;
	const float ratio = w / float(h);
	int resized_w = int(ceil((float)this->inpHeight * ratio));
	if (ceil(this->inpHeight*ratio) > this->inpWidth)
	{
		resized_w = this->inpWidth;
	}

	resize(srcimg, dstimg, cv::Size(resized_w, this->inpHeight), cv::INTER_LINEAR);
	return dstimg;
}

void PaddleTextClassifier::normalize_(cv::Mat img)
{
	//    img.convertTo(img, CV_32F);
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(this->inpHeight * this->inpWidth * img.channels());
	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < inpWidth; j++)
			{
				if (j < col)
				{
					float pix = img.ptr<uchar>(i)[j * 3 + c];
					this->input_image_[c * row * inpWidth + i * inpWidth + j] = (pix / 255.0 - 0.5) / 0.5;
				}
				else
				{
					this->input_image_[c * row * inpWidth + i * inpWidth + j] = 0;
				}
			}
		}
	}
}

int PaddleTextClassifier::predict(cv::Mat cv_image)
{
	cv::Mat dstimg = this->preprocess(cv_image);
	this->normalize_(dstimg);
	std::array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	std::vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size()); 
	const float* pdata = ort_outputs[0].GetTensorMutableData<float>();
	
	int max_id = 0;
	float max_prob = -1;
	for (int i = 0; i < num_out; i++)
	{
		if (pdata[i] > max_prob)
		{
			max_prob = pdata[i];
			max_id = i;
		}
	}

	return max_id;
}