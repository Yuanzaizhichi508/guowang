#include "detectorInterface.h"
#include "paddleTextDet.h"
#include "paddleTextCls.h"
#include "paddleTextRec.h"
#include <timeUtil.hpp>

// Paddle输出帧
class PaddleOutputFrame : public OutputFrameinterface {
public:
    // det结果
    std::vector<std::vector<cv::Point2f>> det_result;

    // rec结果
    std::vector<std::string> rec_result;

    // 平均字高
    float mean_height = 0;

    // 最左边x坐标
    float left_most = 0;

    // 最右边x坐标
    float right_most = 0;

    // 构造函数
    PaddleOutputFrame() {};
    // 构造函数
    // @param det_result: det结果
    PaddleOutputFrame(const std::vector<std::vector<cv::Point2f>>& det_result) : det_result(det_result) {};

    // 更新det结果
    // @param det_result: det结果
    // @param height_threshold: 高度阈值
    // @return 是否成功
    bool UpdateDetResult(const std::vector<std::vector<cv::Point2f>>& det_result, float height_threshold) {
        this->det_result = det_result;
        this->mean_height = 0;
        this->left_most = 0;
        this->right_most = 0;
        for (auto& box : det_result) {
            this->mean_height += box[1].y - box[0].y + box[2].y - box[3].y;
            this->left_most = std::min(this->left_most, box[0].x);
            this->right_most = std::max(this->right_most, box[2].x);
        }
        this->mean_height /= det_result.size() * 2;
        std::sort(this->det_result.begin(), this->det_result.end(), [&](const std::vector<cv::Point2f>& a, const std::vector<cv::Point2f>& b){
            if(abs(a[0].y - b[0].y) < mean_height * height_threshold){
                return a[0].x < b[0].x;
            }
            return a[0].y < b[0].y;
        });
        state |= 1 << 0;
        return true;
    };

    // 更新位置
    // @param position: 位置
    // @return 是否成功
    bool UpdatePosition(PositionType position) {
        this->position = position;
        state |= 1 << 1;
        return true;
    };

    // 更新识别结果
    // @param result: 识别结果
    // @return 是否成功
    bool UpdateResult(const std::string& result) {
        this->result = result;
        state |= 1 << 2;
        return true;
    };
};

// Paddle模块
class PaddleOnnxDetector : public DetectorInterface<PaddleOutputFrame> {
private:
    // 文本检测器
    PaddleTextDetector text_det;
    // 文本识别器
    PaddleTextRecognizer text_rec;
    // 文本分类器
    PaddleTextClassifier text_cls;

    // 水平边距阈值(相对幅面宽度)
    float horizontal_margin;
    // 高度阈值(相对字符平均宽度比例)
    float height_threshold;
    // 反转阈值
    float rev_threshold;

public:
    // 构造函数
    // @param horizontal_margin: 水平边距阈值
    // @param height_threshold: 高度阈值
    // @param rev_threshold: 反转阈值
    PaddleOnnxDetector(
        float horizontal_margin,
        float height_threshold,
        float rev_threshold
    ) : 
        horizontal_margin(horizontal_margin),
        height_threshold(height_threshold),
        rev_threshold(rev_threshold) 
    {
        this->available = 0;
    };

    // 析构函数
    ~PaddleOnnxDetector() override {};

    // 初始化det
    // @param modelPath: 模型路径
    // @param binaryThreshold: 二值化阈值
    // @param polygonThreshold: 多边形阈值
    // @param unclipRatio: unclip比例
    // @param maxCandidates: 最大候选数
    // @param isCuda: 是否使用CUDA
    // @return 是否成功
    bool InitDet(
        const std::string& modelPath,
        float binaryThreshold = 0.3f,
        float polygonThreshold = 0.5f,
        float unclipRatio = 1.6f,
        int maxCandidates = 1000,
        bool isCuda = false
    ) {
        bool res = this->text_det.init(modelPath, binaryThreshold, polygonThreshold, unclipRatio, maxCandidates, isCuda);
        if (res) {
            this->available |= 1 << 0;
        }
        else {
            Log_Error("Failed to load model! Please check the model " + modelPath);
        }
        return res;
    };

    // 初始化cls
    // @param modelPath: 模型路径
    // @param isCuda: 是否使用CUDA
    // @return 是否成功
    bool InitCls(
        const std::string& modelPath,
        bool isCuda = false
    ) {
        bool res = this->text_cls.init(modelPath, isCuda);
        if (res) {
            this->available |= 1 << 1;
        }
        else {
            Log_Error("Failed to load model! Please check the model " + modelPath);
        }
        return res;
    };

    // 初始化rec
    // @param modelPath: 模型路径
    // @param dictPath: 字典路径
    // @param isCuda: 是否使用CUDA
    // @return 是否成功
    bool InitRec(
        const std::string& modelPath,
        const std::string& dictPath,
        bool isCuda = false
    ) {
        bool res = this->text_rec.init(modelPath, dictPath, isCuda);
        if (res) {
            this->available |= 1 << 2;
        }
        else {
            Log_Error("Failed to load model! Please check the model " + modelPath + " and dict " + dictPath);
        }
        return res;
    };

    // 检查是否可用
    // @return 是否可用
    bool CheckAvailable() const override {
        if ((this->available == (1 << 3)) - 1) {
            return true;
        }
        if ((this->available & (1 << 0)) == 0) {
            Log_Warning("Det not loaded!");
        }
        if ((this->available & (1 << 1)) == 0) {
            Log_Warning("Cls not loaded!");
        }
        if ((this->available & (1 << 2)) == 0) {
            Log_Warning("Rec not loaded!");
        }
        return false;
    };

    // 检测
    // @param image: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    bool Detect(const cv::Mat& image, PaddleOutputFrame& output) override {
        if (!CheckAvailable()) {
            return false;
        }
        cv::Mat inputImg = image;
        // 检查通道数
        if (inputImg.channels() != 3) {
            if (inputImg.channels() == 1) {
                Log_Info("[detector] Detect(): Convert to 3 channels!");
                // 灰度图转换为3通道
                cv::cvtColor(inputImg, inputImg, cv::COLOR_GRAY2BGR);
            }
            else {
                Log_Error("[detector] Detect(): Image channels not supported!");
                return false;
            }
        }
        output.UpdateDetResult(text_det.detect(inputImg), height_threshold);
        return true;
    };

    // 检测位置
    // @param image: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    bool DetectPosition(const cv::Mat& image, PaddleOutputFrame& output) override {
        if (!CheckAvailable()) {
            return false;
        }
        if (Detect(image, output)) {
            return _TestPosition(output, image.cols);
        }
        return false;
    };

    // 检测字符串
    // @param image: 输入图像
    // @param output: 输出结果
    // @return 是否成功
    bool DetectString(const cv::Mat& image, PaddleOutputFrame& output) override {
        if (!CheckAvailable()) {
            return false;
        }
        if (Detect(image, output)) {
            return _ExtractString(output, image);
        }
        return false;
    };

    // 处理output, 获取位置
    // @param output: 输出结果
    // @param width: 图像宽度
    // @return 是否成功
    bool _TestPosition(PaddleOutputFrame& output, int width) override {
        // 空的
        if (output.det_result.size() == 0) {
            output.UpdatePosition(PositionType::NONE);
            return true;
        }
        if (output.left_most < this->horizontal_margin * width) {
            output.UpdatePosition(PositionType::LEFT);
            return true;
        }
        if (width - output.right_most < this->horizontal_margin * width) {
            output.UpdatePosition(PositionType::RIGHT);
            return true;
        }
        output.UpdatePosition(PositionType::FINE);
        return true;
    };

    // 处理output, 获取识别结果
    // @param output: 输出结果
    // @param image: 图像
    // @return 是否成功
    bool _ExtractString(PaddleOutputFrame& output, const cv::Mat& image) override {
        if ((output.state & (1 << 0)) == 0 || (this->available & (1 << 2)) == 0) {
            return false;
        }
        if (output.det_result.size() == 0) {
            output.UpdateResult(std::move(std::string()));
            return true;
        }
        std::string result;
        float all_count = 0;
        float rev_count = 0;
        for (int i = 0; i < output.det_result.size(); i++) {
            cv::Mat text_image = text_det.get_rotate_crop_image(image, output.det_result[i]);
            // Log_Debug("=============text_cls " + std::to_string(text_cls.predict(text_image)));
            if (text_cls.predict(text_image) == 1) {
                cv::rotate(text_image, text_image, cv::ROTATE_180);
                rev_count += output.det_result[i][3].x - output.det_result[i][0].x + output.det_result[i][2].x - output.det_result[i][1].x;
            }
            all_count += output.det_result[i][3].x - output.det_result[i][0].x + output.det_result[i][2].x - output.det_result[i][1].x;
            output.rec_result.push_back(text_rec.predict_text(text_image));
        }
        if (rev_count > all_count * rev_threshold) {
            output.reverse = true;
            // 注意还需要在中间加空格
            result = std::accumulate(output.rec_result.rbegin(), output.rec_result.rend(), std::string(), [](std::string sum, const std::string& a) {
                return sum + a + " ";
            });
        }
        else {
            output.reverse = false;
            result = std::accumulate(output.rec_result.begin(), output.rec_result.end(), std::string(), [](std::string sum, const std::string& a) {
                return sum + a + " ";
            });
        }
        if (result.size() > 0) {
            result.pop_back();
        }
        output.UpdateResult(result);
        return true;
    };

    // 绘制结果
    // @param image: 待绘制图像
    // @param output: 输出结果
    void DrawResult(cv::Mat& image, const PaddleOutputFrame& output) override {
        // 检查是否已经完成检测，以及是否存在classes
        if ((output.state & (1 << 0)) == 0 || (this->available & (1 << 1)) == 0) {
            return;
        }
        std::vector<std::vector<cv::Point2f>> det_result(output.det_result);
        if (output.state & (1 << 2)) {
            // 已经检测了字符串，可以获取正反向
            if (
                output.reverse &&
                this->available & (1 << 2)
            ) {
                // 首先反转图片
                cv::flip(image, image, -1);
                // 然后反转结果
                for (auto& out : det_result) {
                    for (auto& point : out) {
                        point.x = image.cols - point.x;
                        point.y = image.rows - point.y;
                    }
                    std::swap(out[0], out[2]);
                    std::swap(out[1], out[3]);
                }
            }
            for (int i = 0; i < det_result.size(); i++) {
                auto box = det_result[i];
                // cv::putText(image, output.rec_result[i], box[1], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                cv::putText(image, "0", box[0], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                cv::putText(image, "1", box[1], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                cv::putText(image, "2", box[2], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                cv::putText(image, "3", box[3], cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
                cv::line(image, box[0], box[1], cv::Scalar(0, 255, 0), 2);
                cv::line(image, box[1], box[2], cv::Scalar(0, 255, 0), 2);
                cv::line(image, box[2], box[3], cv::Scalar(0, 255, 0), 2);
                cv::line(image, box[3], box[0], cv::Scalar(0, 255, 0), 2);
            }
        }
    };
};