#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWaitCondition>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include "ControlThread.h"
#include <iostream>
#include <QTimer>
#include "Common.h"
#include "AcquisitionThread.h"
#include "ClickableLabel.h"
#include "ImageViewer.h"
#include "stampStringMatcher.h"
#include "testUtils.hpp"

#define ENUMRATE_TIME_OUT       200

#ifdef USE_PADDLE
#define TYPE PaddleOutputFrame
#else
#define TYPE YoloOutputFrame
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    friend std::ostream& operator<<(std::ostream& os, const PositionType& position);

    QImage MatToQImage(const cv::Mat& mat);

    void slotShowImage(int camIndex, AcquisitionThread* camAcq, QLabel* label,
                        std::unordered_map<int, std::deque<cv::Mat>>& photoQueue, int& num, bool& isNew,
                        const char* signalName,bool& isCaptured);

    void closeEvent(QCloseEvent *event);

    int levenshtein_distance(const string& s1, const string& s2);

    void computeAccuracy(int streamIndex, std::string &trueString);

signals:
    void firstCamImageCaptured();
    void secondCamImageCaptured();
    void thirdCamImageCaptured();
    void forthCamImageCaptured();
    void TestCamImageCaptured(std::unordered_map<int, std::deque<cv::Mat>>& photoQueue,
                              QComboBox* combox, bool& isNew, int CamID);

private slots:
    void on_PlcConnect_clicked();

    void on_UpdateDeviceList_clicked();

    void on_PlcDisConnect_clicked();

    void on_OpenDevice_clicked();

    void on_TriggerMode_activated(int index);

    void on_TriggerSource_activated(int index);

    void on_TriggerSoftWare_clicked();

    void on_CloseDevice_clicked();

    void on_StartAcquisition_clicked();

    void on_StopAcquisition_clicked();

    void slotShowImage1();

    void slotShowImage2();

    void slotShowImage3();

    void slotShowImage4();

    void slotShowFrameRate(int index);

    void on_StartVerify_clicked();

    void slotSaveFirstCamImage();

    void slotSaveSecondCamImage();

    void slotSaveThirdCamImage();

    void slotSaveForthCamImage();

    void on_StopVerify_clicked();

    void onLogMessage(const QString &message);

    void slotStartFirstCamTask();

    void slotEndFirstCamTask();

    void slotStartSecondCamTask();

    void slotEndSecondCamTask();

    void slotStartThirdCamTask();

    void slotEndThirdCamTask();

    void slotStartForthCamTask();

    void slotEndForthCamTask();

    void FirstCamImagesCapturedAndReady();

    void SecondCamImagesCapturedAndReady();

    void ThirdCamImagesCapturedAndReady();

    void ForthCamImagesCapturedAndReady();

    void TestCamImageCapturedAndReady(std::unordered_map<int, std::deque<cv::Mat>>& photoQueue,
                                      QComboBox* combox, bool& isNew, int CamID);

    void slotUpdateCombox1();

    void slotUpdateCombox2();

    void slotUpdateCombox3();

    void slotUpdateCombox4();

    void on_result_comboBox_1_activated(int index);

    void on_result_comboBox_2_activated(int index);

    void on_result_comboBox_3_activated(int index);

    void on_result_comboBox_4_activated(int index);

    void showZoomedImage1();

    void showZoomedImage2();

    void on_GetModel_comboBox_activated(int index);

    void on_GetModel_comboBox_2_activated(int index);

    void on_GetModel_comboBox_3_activated(int index);

    void on_GetModel_comboBox_4_activated(int index);


    void on_startTestButton_clicked();

    void on_stopTestButton_clicked();


private:
    Ui::MainWindow *ui;

    /*
     * 函数定义
    */
    void UpdateUI();

    void UpdateDeviceList();

    void OpenDevice();

    void CloseDevice();

    void ShowDeviceInfo();

    void GetDeviceInitParam();

    void SetUpAcquisitionThread();

    double GetImageShowFps(int index);

    void SaveImage(cv::Mat image, int imagecount, int dir);

    void updateCombox(int streamIndex, const std::unordered_map<int, int>& lengths, QComboBox* resultComboBox);

    void setupModelComboBox();



    /*
     * 参数定义
    */
    GX_DEV_HANDLE*                     deviceHandles;   //相机设备参数
    GX_DEVICE_BASE_INFO*               m_pstBaseInfo;  //相机设备参数
    CAMER_INFO*                        m_pstCam;       // 相机数据结构体
    CFps*                              m_pCamsFps;     //FPS定义
    int                               m_nOperateID;    // 操作设备ID
    uint32_t                          m_ui32DeviceNum; //设备数目
    AcquisitionThread*                m_CamAcq1;
    AcquisitionThread*                m_CamAcq2;
    AcquisitionThread*                m_CamAcq3;
    AcquisitionThread*                m_CamAcq4;
    QTimer*                           m_ShowFrameRateTimer1;
    QTimer*                           m_ShowFrameRateTimer2;
    QTimer*                           m_ShowFrameRateTimer3;
    QTimer*                           m_ShowFrameRateTimer4;
    ControlThread*                    m_Control;
    cv::Mat                           FirstImg;
    cv::Mat                           SecondImg;
    cv::Mat                           ThirdImg;
    cv::Mat                           ForthImg;
    cv::Mat                           emptyImage = cv::Mat::zeros(1200, 1920, CV_8UC3);




    /*
     * 状态定义
    */
    bool                 m_bTriggerModeOn;  //
    bool                 m_bSoftTriggerOn;  //
    bool                 is_Start;
    bool                 is_Cap1 = true;
    bool                 is1 = false;
    bool                 is2 = false;
    bool                 is3 = false;
    bool                 is4 = false;
    bool                 isTest = false;

    /*
     * 检测相关
     */
    bool isCuda = Config::GetDevice() == "cuda" || Config::GetDevice() == "gpu";
    std::string modelPath;
    std::string classesPath;
    std::string reverseMapPath;
    StampStringMatcher matcher;
    TestStampDataset testDataset;

    QTimer*    SampleTimer1;
    QTimer*    SampleTimer2;
    QTimer*    SampleTimer3;
    QTimer*    SampleTimer4;
    int        SampleInterval = 100;//拍照采样时间


    int sectionNum = 4;
    int imageCaptureCount = 0;
#ifndef USE_PADDLE
    std::vector<YoloOnnxDetector*> detectors;
#else
    std::vector<PaddleOnnxDetector*> detectors;
#endif

    std::vector<DetectStream<TYPE>*> detectStreams;
    int idx1 = 0;
    int idx2 = 0;
    int idx3 = 0;
    int idx4 = 0;
    std::mutex ui_mutex;
    std::mutex task_mutex;
    std::mutex mtx1, mtx2, mtx3, mtx4;
    int currentTaskID = -1;


    int count1 = 0;
    int count2 = 0;
    QString imgPath1;
    QString imgPath2;
};
#endif // MAINWINDOW_H
