#include "paddleTextDet.h"
#include <filesystem>

using namespace Ort;

PaddleTextDetector::PaddleTextDetector(){}

PaddleTextDetector::~PaddleTextDetector()
{
	if(ort_session != nullptr){
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

bool PaddleTextDetector::init(
	const std::string& modelPath,
	float binaryThreshold,
	float polygonThreshold,
	float unclipRatio,
	int maxCandidates,
	bool isCuda
){
	// 检查模型文件是否存在，是否为onnx文件
	if (
		!std::filesystem::exists(modelPath) ||
		modelPath.substr(modelPath.find_last_of(".") + 1) != "onnx"
	) {
		return false;
	}

	this->binaryThreshold = binaryThreshold;
	this->polygonThreshold = polygonThreshold;
	this->unclipRatio = unclipRatio;
	this->maxCandidates = maxCandidates;
	
	try{
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
		}
		for (int i = 0; i < numOutputNodes; i++)
		{
			char* copied_output_name = new char[strlen(ort_session->GetOutputNameAllocated(i, allocator).get()) + 1];
			strcpy(copied_output_name, ort_session->GetOutputNameAllocated(i, allocator).get());
			output_names.push_back(copied_output_name);
		}
	}
	catch(const std::exception& e){
		return false;
	}
	return true;
}

cv::Mat PaddleTextDetector::preprocess(cv::Mat srcimg)
{
	cv::Mat dstimg;
	cvtColor(srcimg, dstimg, cv::COLOR_BGR2RGB);
	int h = srcimg.rows;
	int w = srcimg.cols;
	float scale_h = 1;
	float scale_w = 1;
	if (h < w)
	{
		scale_h = (float)this->short_size / (float)h;
		float tar_w = (float)w * scale_h;
		tar_w = tar_w - (int)tar_w % 32;
		tar_w = std::max((float)32, tar_w);
		scale_w = tar_w / (float)w;
	}
	else
	{
		scale_w = (float)this->short_size / (float)w;
		float tar_h = (float)h * scale_w;
		tar_h = tar_h - (int)tar_h % 32;
		tar_h = std::max((float)32, tar_h);
		scale_h = tar_h / (float)h;
	}
	resize(dstimg, dstimg, cv::Size(int(scale_w*dstimg.cols), int(scale_h*dstimg.rows)), cv::INTER_LINEAR);
	return dstimg;
}

void PaddleTextDetector::normalize_(cv::Mat img)
{
	//    img.convertTo(img, CV_32F);
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(row * col * img.channels());
	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				float pix = img.ptr<uchar>(i)[j * 3 + c];
				this->input_image_[c * row * col + i * col + j] = (pix / 255.0 - this->meanValues[c]) / this->normValues[c];
			}
		}
	}
}

std::vector<std::vector<cv::Point2f>> PaddleTextDetector::detect(cv::Mat& srcimg)
{
	int h = srcimg.rows;
	int w = srcimg.cols;
	cv::Mat dstimg = this->preprocess(srcimg);
	this->normalize_(dstimg);
	std::array<int64_t, 4> input_shape_{ 1, 3, dstimg.rows, dstimg.cols };
	
	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	std::vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size());
	const float* floatArray = ort_outputs[0].GetTensorMutableData<float>();
	int outputCount = 1;
	for(int i=0; i < ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().size(); i++)
	{
		int dim = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(i);
		outputCount *= dim;
	}

	cv::Mat binary(dstimg.rows, dstimg.cols, CV_32FC1);
	memcpy(binary.data, floatArray, outputCount * sizeof(float));

	// Threshold
	cv::Mat bitmap;
	threshold(binary, bitmap, binaryThreshold, 255, cv::THRESH_BINARY);
	// Scale ratio
	float scaleHeight = (float)(h) / (float)(binary.size[0]);
	float scaleWidth = (float)(w) / (float)(binary.size[1]);
	// Find contours
	std::vector< std::vector<cv::Point> > contours;
	bitmap.convertTo(bitmap, CV_8UC1);
	findContours(bitmap, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	// Candidate number limitation
	size_t numCandidate = std::min(contours.size(), (size_t)(maxCandidates > 0 ? maxCandidates : INT_MAX));
	std::vector<float> confidences;
	std::vector< std::vector<cv::Point2f> > results;
	for (size_t i = 0; i < numCandidate; i++)
	{
		std::vector<cv::Point>& contour = contours[i];

		// Calculate text contour score
		if (contourScore(binary, contour) < polygonThreshold)
			continue;

		// Rescale
		std::vector<cv::Point> contourScaled; contourScaled.reserve(contour.size());
		for (size_t j = 0; j < contour.size(); j++)
		{
			contourScaled.push_back(cv::Point(int(contour[j].x * scaleWidth),
				int(contour[j].y * scaleHeight)));
		}

		// Unclip
		cv::RotatedRect box = minAreaRect(contourScaled);
		float longSide = std::max(box.size.width, box.size.height);
		if (longSide < longSideThresh) 
		{
			continue;
		}

		// minArea() rect is not normalized, it may return rectangles with angle=-90 or height < width
		const float angle_threshold = 60;  // do not expect vertical text, TODO detection algo property
		bool swap_size = false;
		if (box.size.width < box.size.height)  // horizontal-wide text area is expected
			swap_size = true;
		else if (fabs(box.angle) >= angle_threshold)  // don't work with vertical rectangles
			swap_size = true;
		if (swap_size)
		{
			std::swap(box.size.width, box.size.height);
			if (box.angle < 0)
				box.angle += 90;
			else if (box.angle > 0)
				box.angle -= 90;
		}

		cv::Point2f vertex[4];
		box.points(vertex);  // order: bl, tl, tr, br
		std::vector<cv::Point2f> approx;
		for (int j = 0; j < 4; j++)
			approx.emplace_back(vertex[j]);
		std::vector<cv::Point2f> polygon;
		unclip(approx, polygon);

		box = minAreaRect(polygon);
		longSide = std::max(box.size.width, box.size.height);
		if (longSide < longSideThresh+2)
		{
			continue;
		}

		results.push_back(polygon);
	}
	confidences = std::vector<float>(contours.size(), 1.0f);
	return results;
	/*std::vector< std::vector<cv::Point2f> > order_points = this->order_points_clockwise(results);
	return order_points;*/
}

std::vector< std::vector<cv::Point2f> > PaddleTextDetector::order_points_clockwise(std::vector< std::vector<cv::Point2f> > results)
{
	std::vector< std::vector<cv::Point2f> > order_points(results);
	for (int i = 0; i < results.size(); i++)
	{
		float max_sum_pts = -10000;
		float min_sum_pts = 10000;
		float max_diff_pts = -10000;
		float min_diff_pts = 10000;

		int max_sum_pts_id = 0;
		int min_sum_pts_id = 0;
		int max_diff_pts_id = 0;
		int min_diff_pts_id = 0;
		for (int j = 0; j < 4; j++)
		{
			const float sum_pt = results[i][j].x + results[i][j].y;
			if (sum_pt > max_sum_pts)
			{
				max_sum_pts = sum_pt;
				max_sum_pts_id = j;
			}
			if (sum_pt < min_sum_pts)
			{
				min_sum_pts = sum_pt;
				min_sum_pts_id = j;
			}

			const float diff_pt = results[i][j].y - results[i][j].x;
			if (diff_pt > max_diff_pts)
			{
				max_diff_pts = diff_pt;
				max_diff_pts_id = j;
			}
			if (diff_pt < min_diff_pts)
			{
				min_diff_pts = diff_pt;
				min_diff_pts_id = j;
			}
		}
		order_points[i][0].x = results[i][min_sum_pts_id].x;
		order_points[i][0].y = results[i][min_sum_pts_id].y;
		order_points[i][2].x = results[i][max_sum_pts_id].x;
		order_points[i][2].y = results[i][max_sum_pts_id].y;

		order_points[i][1].x = results[i][min_diff_pts_id].x;
		order_points[i][1].y = results[i][min_diff_pts_id].y;
		order_points[i][3].x = results[i][max_diff_pts_id].x;
		order_points[i][3].y = results[i][max_diff_pts_id].y;
	}
	return order_points;
}

void PaddleTextDetector::draw_pred(cv::Mat& srcimg, std::vector< std::vector<cv::Point2f> > results)
{
	for (int i = 0; i < results.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			circle(srcimg, cv::Point((int)results[i][j].x, (int)results[i][j].y), 2, cv::Scalar(0, 0, 255), -1);
			if (j < 3)
			{
				line(srcimg, cv::Point((int)results[i][j].x, (int)results[i][j].y), cv::Point((int)results[i][j + 1].x, (int)results[i][j + 1].y), cv::Scalar(0, 255, 0));
			}
			else
			{
				line(srcimg, cv::Point((int)results[i][j].x, (int)results[i][j].y), cv::Point((int)results[i][0].x, (int)results[i][0].y), cv::Scalar(0, 255, 0));
			}
		}
	}
}

float PaddleTextDetector::contourScore(const cv::Mat& binary, const std::vector<cv::Point>& contour)
{
	cv::Rect rect = boundingRect(contour);
	int xmin = std::max(rect.x, 0);
	int xmax = std::min(rect.x + rect.width, binary.cols - 1);
	int ymin = std::max(rect.y, 0);
	int ymax = std::min(rect.y + rect.height, binary.rows - 1);

	cv::Mat binROI = binary(cv::Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1));

	cv::Mat mask = cv::Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8U);
	std::vector<cv::Point> roiContour;
	for (size_t i = 0; i < contour.size(); i++) {
		cv::Point pt = cv::Point(contour[i].x - xmin, contour[i].y - ymin);
		roiContour.push_back(pt);
	}
	std::vector<std::vector<cv::Point>> roiContours = { roiContour };
	fillPoly(mask, roiContours, cv::Scalar(1));
	float score = mean(binROI, mask).val[0];
	return score;
}

void PaddleTextDetector::unclip(const std::vector<cv::Point2f>& inPoly, std::vector<cv::Point2f> &outPoly)
{
	float area = contourArea(inPoly);
	float length = arcLength(inPoly, true);
	float distance = area * unclipRatio / length;

	size_t numPoints = inPoly.size();
	std::vector<std::vector<cv::Point2f>> newLines;
	for (size_t i = 0; i < numPoints; i++)
	{
		std::vector<cv::Point2f> newLine;
		cv::Point pt1 = inPoly[i];
		cv::Point pt2 = inPoly[(i - 1) % numPoints];
		cv::Point vec = pt1 - pt2;
		float unclipDis = (float)(distance / norm(vec));
		cv::Point2f rotateVec = cv::Point2f(vec.y * unclipDis, -vec.x * unclipDis);
		newLine.push_back(cv::Point2f(pt1.x + rotateVec.x, pt1.y + rotateVec.y));
		newLine.push_back(cv::Point2f(pt2.x + rotateVec.x, pt2.y + rotateVec.y));
		newLines.push_back(newLine);
	}

	size_t numLines = newLines.size();
	for (size_t i = 0; i < numLines; i++)
	{
		cv::Point2f a = newLines[i][0];
		cv::Point2f b = newLines[i][1];
		cv::Point2f c = newLines[(i + 1) % numLines][0];
		cv::Point2f d = newLines[(i + 1) % numLines][1];
		cv::Point2f pt;
		cv::Point2f v1 = b - a;
		cv::Point2f v2 = d - c;
		float cosAngle = (v1.x * v2.x + v1.y * v2.y) / (norm(v1) * norm(v2));

		if (fabs(cosAngle) > 0.7)
		{
			pt.x = (b.x + c.x) * 0.5;
			pt.y = (b.y + c.y) * 0.5;
		}
		else
		{
			float denom = a.x * (float)(d.y - c.y) + b.x * (float)(c.y - d.y) +
				d.x * (float)(b.y - a.y) + c.x * (float)(a.y - b.y);
			float num = a.x * (float)(d.y - c.y) + c.x * (float)(a.y - d.y) + d.x * (float)(c.y - a.y);
			float s = num / denom;

			pt.x = a.x + s * (b.x - a.x);
			pt.y = a.y + s * (b.y - a.y);
		}
		outPoly.push_back(pt);
	}
}

cv::Mat PaddleTextDetector::get_rotate_crop_image(const cv::Mat& frame, std::vector<cv::Point2f> vertices)
{
	cv::Rect rect = boundingRect(cv::Mat(vertices));
	cv::Mat crop_img = frame(rect);

	const cv::Size outputSize = cv::Size(rect.width, rect.height);

	std::vector<cv::Point2f> targetVertices{ cv::Point2f(0, outputSize.height),cv::Point2f(0, 0), cv::Point2f(outputSize.width, 0), cv::Point2f(outputSize.width, outputSize.height)};
	//std::vector<cv::Point2f> targetVertices{ cv::Point2f(0, 0), cv::Point2f(outputSize.width, 0), cv::Point2f(outputSize.width, outputSize.height), cv::Point2f(0, outputSize.height) };

	for (int i = 0; i < 4; i++)
	{
		vertices[i].x -= rect.x;
		vertices[i].y -= rect.y;
	}
	
	cv::Mat rotationMatrix = getPerspectiveTransform(vertices, targetVertices);
	cv::Mat result;
	warpPerspective(crop_img, result, rotationMatrix, outputSize, cv::BORDER_REPLICATE);
	return result;
}