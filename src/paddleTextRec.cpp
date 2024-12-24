#include "paddleTextRec.h"
#include <filesystem>

using namespace Ort;

PaddleTextRecognizer::PaddleTextRecognizer(){}

PaddleTextRecognizer::~PaddleTextRecognizer()
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

bool PaddleTextRecognizer::init(
	const std::string& modelPath,
	const std::string& dictPath,
	bool isCuda
) {
	// 检查模型文件是否存在，是否为onnx文件
	// 检查字典文件是否存在，是否为txt文件
	if (
		!std::filesystem::exists(modelPath) ||
		modelPath.substr(modelPath.find_last_of(".") + 1) != "onnx" ||
		!std::filesystem::exists(dictPath) ||
		dictPath.substr(dictPath.find_last_of(".") + 1) != "txt"
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

		// 读取字典文件
		std::ifstream ifs(dictPath);
		std::string line;
		while (std::getline(ifs, line))
		{
			this->alphabet.push_back(line);
		}
		this->alphabet.push_back(" ");
		names_len = this->alphabet.size();
	}
	catch (const std::exception& e) {
		return false;
	}
	return true;
}

cv::Mat PaddleTextRecognizer::preprocess(cv::Mat srcimg)
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
	
	cv::resize(srcimg, dstimg, cv::Size(resized_w, this->inpHeight), cv::INTER_LINEAR);
	return dstimg;
}

void PaddleTextRecognizer::normalize_(cv::Mat img)
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

std::string PaddleTextRecognizer::predict_text(cv::Mat cv_image)
{
	cv::Mat dstimg = this->preprocess(cv_image);
	this->normalize_(dstimg);
	
	std::array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	// 开始推理
	std::vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size());   // 开始推理
	
	const float* pdata = ort_outputs[0].GetTensorMutableData<float>();
	
	int i = 0, j = 0;
	int h = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(2);
	int w = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(1);
	
	preb_label.resize(w);
	for (i = 0; i < w; i++)
	{
		int one_label_idx = 0;
		float max_data = -10000;
		for (j = 0; j < h; j++)
		{
			float data_ = pdata[i*h + j];
			if (data_ > max_data)
			{
				max_data = data_;
				one_label_idx = j;
			}
		}
		preb_label[i] = one_label_idx;
	}
	
	std::vector<int> no_repeat_blank_label;
	for (size_t elementIndex = 0; elementIndex < w; ++elementIndex)
	{
		if (preb_label[elementIndex] != 0 && !(elementIndex > 0 && preb_label[elementIndex - 1] == preb_label[elementIndex]))
		{
			no_repeat_blank_label.push_back(preb_label[elementIndex] - 1);
		}
	}
	
	int len_s = no_repeat_blank_label.size();
	std::string plate_text;
	for (i = 0; i < len_s; i++)
	{
		plate_text += alphabet[no_repeat_blank_label[i]];
	}
	
	return plate_text;
}
