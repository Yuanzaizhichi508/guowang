#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include "ControlThread.h"
#include <memory>
#include <QThread>
#include <QMutex>
#include <deque>
#include <QDebug>
#include "Common.h"
#include "globalvariable.h"

#include "Fps.h"

enum PROC_STATUS
{
    PROC_SUCCESS = 0,
    PROC_FAIL = -1
};

class AcquisitionThread :public QThread
{
    Q_OBJECT
public:
    AcquisitionThread();

    void GetParam(GX_DEV_HANDLE*,CAMER_INFO*,int);

    void PushBackToMatDeque(cv::Mat);

    void PushBackToShowImageDeque(std::shared_ptr<QImage>);

    std::shared_ptr<QImage> PopFrontFromShowImageDeque();

    cv::Mat PopFrontFromMatDeque();

    std::shared_ptr<QImage> GetFrontShowImageDeque();

    void ClearDeque();

    void RegisterCallback(int nCamID);

    double GetImageAcqFps();

    uint32_t            m_nFrameCount = 0;

signals:
       void SigGetSuccess1();
       void SigGetSuccess2();
       void SigGetSuccess3();
       void SigGetSuccess4();

protected:
    void run();

private:
    GX_DEV_HANDLE*       deviceHandles;
    CAMER_INFO*          m_pstCam;       // 相机数据结构体
    int                  m_nOperateID;
    std::deque<cv::Mat> m_objMatBufferDeque;
    std::deque<std::shared_ptr<QImage>> m_objShowImageDeque;
    QMutex              m_objMatMutex;
    QMutex              m_objDequeMutex;
    CFps                m_objFps;

    static void __stdcall OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* pFrame);
    static void __stdcall OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* pFrame);
    static void __stdcall OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* pFrame);
    static void __stdcall OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* pFrame);
};

#endif // ACQUISITIONTHREAD_H
