#include "DetectThread.h"

// 重载流输出运算符
std::ostream& operator<<(std::ostream& os, const PositionType& position) {
    switch (position) {
    case PositionType::NONE: os << "NONE"; break;
    case PositionType::FINE: os << "FINE"; break;
    case PositionType::LEFT: os << "LEFT"; break;
    case PositionType::RIGHT: os << "RIGHT"; break;
    }
    return os;
}

DetectThread::DetectThread():
    detector(
          Config::GetMinSamples(),
          Config::GetConf(),
          Config::GetNmsThreshold(),
          Config::GetWidthThreshold(),
          Config::GetHeightThreshold(),
          Config::GetRevThreshold()
          )
{
    modelPath = Config::GetModelPath();
    classesPath = Config::GetClassesListPath();
    reverseMapPath = Config::GetReverseMapPath();
    bool isCuda = Config::GetDevice() == "cuda";
    if(detector.LoadModel(modelPath, isCuda)){
        qDebug() << "model loaded";
    }
    if(detector.LoadClasses(classesPath)){
        qDebug() << "classes loaded";
    }
    if(detector.LoadReverseMap(reverseMapPath)){
        qDebug() << "reverse map loaded";
    }
}

void DetectThread::run()
{
    int idx = 0;
    // 这个回调用于实时显示检测结果
    auto onResult = [&](cv::Mat image, OutputFrame output){
        QString idxMessage = QString("====onResult: %1 ====").arg(idx);
        emit logMessage(idxMessage);
        // 画出检测结果
        detector.DrawBoxes(image, output);
        cv::imwrite("images/" + std::to_string(idx++) + ".jpg", image);
    };
    std::vector<DetectSection*> detectSections(sectionNum);
    for(int i = 0; i < sectionNum; i++){
        detectSections[i] = new DetectSection(detector);
        detectSections[i]->Start();
    }
    std::vector<DetectStream*> detectStreams(sectionNum);
    for(int i = 0; i < sectionNum; i++){
        detectStreams[i] = new DetectStream(*(detectSections[i]), onResult);
        detectStreams[i]->Start();
    }
    std::chrono::time_point<std::chrono::system_clock> start, end;
    //正向运动
    while(true){
        if(GlobalVariable::GetInstance().isNew){
            // detectStreams[0]->StartTask();
            // while(!GlobalVariable::GetInstance().ImageForwardQueue[operateID].empty()){
            //     start = std::chrono::system_clock::now();

            //     cv::Mat sectionImageForward = QImageToCvMat(GlobalVariable::GetInstance().ImageForwardQueue[operateID].front());
            //     GlobalVariable::GetInstance().ImageForwardQueue[operateID].pop_front();
            //     cv::Mat sectionImageBackward = QImageToCvMat(GlobalVariable::GetInstance().ImageBackwardQueue[operateID].front());
            //     GlobalVariable::GetInstance().ImageBackwardQueue[operateID].pop_front();
            //     detectStreams[0]->AddImage(std::move(sectionImageForward), std::move(sectionImageBackward));

            //     end = std::chrono::system_clock::now();
            //     std::chrono::duration<double> elapsed_seconds = end - start;
            //     int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count();

            //     if(elapsed_milliseconds < sampleInterval){
            //         Sleep(sampleInterval - elapsed_milliseconds);
            //     }
            // }
            // detectStreams[0]->EndTask();
            // Sleep(sampleInterval);
        }
        // cv::Mat img;
        // OutputFrame output;
        // while(detectStreams[0]->GetFrontResult(img, output)){
        //     QString resultMessage = QString::fromStdString(output.result);
        //     emit logMessage(resultMessage);
        //     std::ostringstream oss;
        //     oss << output.position;
        //     QString positionMessage = QString::fromStdString(oss.str());
        //     emit logMessage(positionMessage);
        //     std::cout << output.position << std::endl;
        // }
    }
}

cv::Mat DetectThread::QImageToCvMat(QImage& image)
{
    cv::Mat mat;
    //qDebug() << image.format();
    switch (image.format())
    {
    case QImage::Format_ARGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        //cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        //cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}



