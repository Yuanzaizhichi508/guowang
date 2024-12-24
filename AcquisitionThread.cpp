#include "AcquisitionThread.h"

AcquisitionThread::AcquisitionThread() {}

void AcquisitionThread::run(){
    GX_STATUS   emStatus = GX_STATUS_SUCCESS;
    RegisterCallback(m_nOperateID);

    //设置流层Buffer处理模式
    emStatus = GXSetEnum(deviceHandles[m_nOperateID], GX_DS_ENUM_STREAM_BUFFER_HANDLING_MODE, GX_DS_STREAM_BUFFER_HANDLING_MODE_OLDEST_FIRST);
    //开始采集
    emStatus = GXSendCommand(deviceHandles[m_nOperateID], GX_COMMAND_ACQUISITION_START);
    GX_VERIFY(emStatus);
    if(m_pstCam[m_nOperateID].bIsSnap = false){
        m_objFps.Reset();
        ClearDeque();
    }
}

void AcquisitionThread::GetParam(GX_DEV_HANDLE* hDeviceHandle,CAMER_INFO* camHandle, int OperateID)
{
    deviceHandles = hDeviceHandle;
    m_pstCam = camHandle;
    m_nOperateID = OperateID;
}

void AcquisitionThread::RegisterCallback(int nCamID){
    GX_STATUS emStatus = GX_STATUS_ERROR;
    switch (nCamID) {
    case 0:
        //注册回调
        emStatus = GXRegisterCaptureCallback(deviceHandles[nCamID], this, OnFrameCallbackFun1);
        GX_VERIFY(emStatus);
        break;
    case 1:
        emStatus = GXRegisterCaptureCallback(deviceHandles[nCamID], this, OnFrameCallbackFun2);
        GX_VERIFY(emStatus);
        break;
    case 2:
        emStatus = GXRegisterCaptureCallback(deviceHandles[nCamID], this, OnFrameCallbackFun3);
        GX_VERIFY(emStatus);
        break;
    case 3:
        emStatus = GXRegisterCaptureCallback(deviceHandles[nCamID], this, OnFrameCallbackFun4);
        GX_VERIFY(emStatus);
        break;
    default:
        break;
    }

}

void __stdcall AcquisitionThread::OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* pFrame){
    if (pFrame->status != 0)
    {
        return;
    }

    AcquisitionThread  *pf = (AcquisitionThread*)(pFrame->pUserParam);

    BYTE	    *pImageBuffer = NULL;          //转换后的图像buffer
    BYTE        *pRawBuf      = NULL;          //转换前图像
    int64_t	    nBayerLayout  = 0;             //Bayer格式
    int		    nImgWidth     = 0;             //图像的宽
    int         nImgHeight    = 0;             //图像的高
    int         i = 0;


    nImgWidth    = (int)(pf->m_pstCam[pf->m_nOperateID].nImageWidth);
    nImgHeight   = (int)(pf->m_pstCam[pf->m_nOperateID].nImageHeight);
    pImageBuffer = pf->m_pstCam[pf->m_nOperateID].pImageBuffer;
    pRawBuf      = pf->m_pstCam[pf->m_nOperateID].pRawBuffer;
    nBayerLayout = pf->m_pstCam[pf->m_nOperateID].nBayerLayout;

    memcpy(pf->m_pstCam[pf->m_nOperateID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[pf->m_nOperateID].nPayLoadSise));

    //图像转化
    if(pf->m_pstCam[pf->m_nOperateID].bIsColorFilter)
    {
        //彩色相机需要经过RGB转换
        DxRaw8toRGB24(pRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);

    }
    else
    {
        //黑白图象需要图象翻转
        for(i = 0; i < nImgHeight; i++)
        {
            memcpy((pImageBuffer + i * nImgWidth), (pRawBuf + ( nImgHeight - i - 1) * nImgWidth), nImgWidth);
        }
    }

    std::shared_ptr<QImage> pobjImageBuffer;
    cv::Mat matImage;

    if (pf->m_pstCam[pf->m_nOperateID].bIsColorFilter) {
        // 彩色相机，明确指定使用RGB888格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth * 3, QImage::Format_RGB888);
        matImage = cv::Mat(nImgHeight, nImgWidth, CV_8UC3, pImageBuffer);
        if (pobjImageBuffer->format() != QImage::Format_RGB888) {
            // 如果格式不正确，转换为RGB888
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_RGB888);
        }
    } else {
        // 黑白相机，明确指定使用Grayscale8格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth, QImage::Format_Grayscale8);

        cv::Mat grayMat = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, pImageBuffer);
        cv::cvtColor(grayMat,matImage,cv::COLOR_GRAY2BGR);
        if (pobjImageBuffer->format() != QImage::Format_Grayscale8) {
            // 如果格式不正确，转换为Grayscale8
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_Grayscale8);
        }
    }
    * pobjImageBuffer = pobjImageBuffer->mirrored(false, true);
    cv::flip(matImage, matImage, 0);
    pf->PushBackToShowImageDeque(pobjImageBuffer);
    pf->PushBackToMatDeque(matImage);
    pf->m_nFrameCount++;
    pf->m_objFps.IncreaseFrameNum();
    emit pf->SigGetSuccess1();
}

void __stdcall AcquisitionThread::OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* pFrame){
    if (pFrame->status != 0)
    {
        return;
    }

    AcquisitionThread  *pf = (AcquisitionThread*)(pFrame->pUserParam);

    BYTE	    *pImageBuffer = NULL;          //转换后的图像buffer
    BYTE        *pRawBuf      = NULL;          //转换前图像
    int64_t	    nBayerLayout  = 0;             //Bayer格式
    int		    nImgWidth     = 0;             //图像的宽
    int         nImgHeight    = 0;             //图像的高
    int         i = 0;


    nImgWidth    = (int)(pf->m_pstCam[pf->m_nOperateID].nImageWidth);
    nImgHeight   = (int)(pf->m_pstCam[pf->m_nOperateID].nImageHeight);
    pImageBuffer = pf->m_pstCam[pf->m_nOperateID].pImageBuffer;
    pRawBuf      = pf->m_pstCam[pf->m_nOperateID].pRawBuffer;
    nBayerLayout = pf->m_pstCam[pf->m_nOperateID].nBayerLayout;

    memcpy(pf->m_pstCam[pf->m_nOperateID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[pf->m_nOperateID].nPayLoadSise));

    //图像转化
    if(pf->m_pstCam[pf->m_nOperateID].bIsColorFilter)
    {
        //彩色相机需要经过RGB转换
        DxRaw8toRGB24(pRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);

    }
    else
    {
        //黑白图象需要图象翻转
        for(i = 0; i < nImgHeight; i++)
        {
            memcpy((pImageBuffer + i * nImgWidth), (pRawBuf + ( nImgHeight - i - 1) * nImgWidth), nImgWidth);
        }
    }

    std::shared_ptr<QImage> pobjImageBuffer;
    cv::Mat matImage;

    if (pf->m_pstCam[pf->m_nOperateID].bIsColorFilter) {
        // 彩色相机，明确指定使用RGB888格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth * 3, QImage::Format_RGB888);
        matImage = cv::Mat(nImgHeight, nImgWidth, CV_8UC3, pImageBuffer);
        if (pobjImageBuffer->format() != QImage::Format_RGB888) {
            // 如果格式不正确，转换为RGB888
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_RGB888);
        }
    } else {
        // 黑白相机，明确指定使用Grayscale8格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth, QImage::Format_Grayscale8);
        cv::Mat grayMat = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, pImageBuffer);
        cv::cvtColor(grayMat,matImage,cv::COLOR_GRAY2BGR);
        if (pobjImageBuffer->format() != QImage::Format_Grayscale8) {
            // 如果格式不正确，转换为Grayscale8
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_Grayscale8);
        }
    }

    * pobjImageBuffer = pobjImageBuffer->mirrored(false, true);
    cv::flip(matImage, matImage, 0);
    pf->PushBackToShowImageDeque(pobjImageBuffer);
    pf->PushBackToMatDeque(matImage);
    pf->m_nFrameCount++;
    pf->m_objFps.IncreaseFrameNum();
    emit pf->SigGetSuccess2();
}

void __stdcall AcquisitionThread::OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* pFrame){
    if (pFrame->status != 0)
    {
        return;
    }

    AcquisitionThread  *pf = (AcquisitionThread*)(pFrame->pUserParam);

    BYTE	    *pImageBuffer = NULL;          //转换后的图像buffer
    BYTE        *pRawBuf      = NULL;          //转换前图像
    int64_t	    nBayerLayout  = 0;             //Bayer格式
    int		    nImgWidth     = 0;             //图像的宽
    int         nImgHeight    = 0;             //图像的高
    int         i = 0;


    nImgWidth    = (int)(pf->m_pstCam[pf->m_nOperateID].nImageWidth);
    nImgHeight   = (int)(pf->m_pstCam[pf->m_nOperateID].nImageHeight);
    pImageBuffer = pf->m_pstCam[pf->m_nOperateID].pImageBuffer;
    pRawBuf      = pf->m_pstCam[pf->m_nOperateID].pRawBuffer;
    nBayerLayout = pf->m_pstCam[pf->m_nOperateID].nBayerLayout;

    memcpy(pf->m_pstCam[pf->m_nOperateID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[pf->m_nOperateID].nPayLoadSise));

    //图像转化
    if(pf->m_pstCam[pf->m_nOperateID].bIsColorFilter)
    {
        //彩色相机需要经过RGB转换
        DxRaw8toRGB24(pRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);

    }
    else
    {
        //黑白图象需要图象翻转
        for(i = 0; i < nImgHeight; i++)
        {
            memcpy((pImageBuffer + i * nImgWidth), (pRawBuf + ( nImgHeight - i - 1) * nImgWidth), nImgWidth);
        }
    }

    std::shared_ptr<QImage> pobjImageBuffer;
    cv::Mat matImage;

    if (pf->m_pstCam[pf->m_nOperateID].bIsColorFilter) {
        // 彩色相机，明确指定使用RGB888格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth * 3, QImage::Format_RGB888);
        matImage = cv::Mat(nImgHeight, nImgWidth, CV_8UC3, pImageBuffer);
        if (pobjImageBuffer->format() != QImage::Format_RGB888) {
            // 如果格式不正确，转换为RGB888
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_RGB888);
        }
    } else {
        // 黑白相机，明确指定使用Grayscale8格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth, QImage::Format_Grayscale8);
        cv::Mat grayMat = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, pImageBuffer);
        cv::cvtColor(grayMat,matImage,cv::COLOR_GRAY2BGR);
        if (pobjImageBuffer->format() != QImage::Format_Grayscale8) {
            // 如果格式不正确，转换为Grayscale8
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_Grayscale8);
        }
    }

    * pobjImageBuffer = pobjImageBuffer->mirrored(false, true);
    cv::flip(matImage, matImage, 0);
    pf->PushBackToShowImageDeque(pobjImageBuffer);
    pf->PushBackToMatDeque(matImage);
    pf->m_nFrameCount++;
    pf->m_objFps.IncreaseFrameNum();
    emit pf->SigGetSuccess3();
}

void __stdcall AcquisitionThread::OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* pFrame){
    if (pFrame->status != 0)
    {
        return;
    }

    AcquisitionThread  *pf = (AcquisitionThread*)(pFrame->pUserParam);

    BYTE	    *pImageBuffer = NULL;          //转换后的图像buffer
    BYTE        *pRawBuf      = NULL;          //转换前图像
    int64_t	    nBayerLayout  = 0;             //Bayer格式
    int		    nImgWidth     = 0;             //图像的宽
    int         nImgHeight    = 0;             //图像的高
    int         i = 0;


    nImgWidth    = (int)(pf->m_pstCam[pf->m_nOperateID].nImageWidth);
    nImgHeight   = (int)(pf->m_pstCam[pf->m_nOperateID].nImageHeight);
    pImageBuffer = pf->m_pstCam[pf->m_nOperateID].pImageBuffer;
    pRawBuf      = pf->m_pstCam[pf->m_nOperateID].pRawBuffer;
    nBayerLayout = pf->m_pstCam[pf->m_nOperateID].nBayerLayout;

    memcpy(pf->m_pstCam[pf->m_nOperateID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[pf->m_nOperateID].nPayLoadSise));

    //图像转化
    if(pf->m_pstCam[pf->m_nOperateID].bIsColorFilter)
    {
        //彩色相机需要经过RGB转换
        DxRaw8toRGB24(pRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);

    }
    else
    {
        //黑白图象需要图象翻转
        for(i = 0; i < nImgHeight; i++)
        {
            memcpy((pImageBuffer + i * nImgWidth), (pRawBuf + ( nImgHeight - i - 1) * nImgWidth), nImgWidth);
        }
    }

    std::shared_ptr<QImage> pobjImageBuffer;
    cv::Mat matImage;

    if (pf->m_pstCam[pf->m_nOperateID].bIsColorFilter) {
        // 彩色相机，明确指定使用RGB888格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth * 3, QImage::Format_RGB888);
        matImage = cv::Mat(nImgHeight, nImgWidth, CV_8UC3, pImageBuffer);
        if (pobjImageBuffer->format() != QImage::Format_RGB888) {
            // 如果格式不正确，转换为RGB888
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_RGB888);
        }
    } else {
        // 黑白相机，明确指定使用Grayscale8格式
        pobjImageBuffer = std::make_shared<QImage>(pImageBuffer, nImgWidth, nImgHeight, nImgWidth, QImage::Format_Grayscale8);
        cv::Mat grayMat = cv::Mat(nImgHeight, nImgWidth, CV_8UC1, pImageBuffer);
        cv::cvtColor(grayMat,matImage,cv::COLOR_GRAY2BGR);
        if (pobjImageBuffer->format() != QImage::Format_Grayscale8) {
            // 如果格式不正确，转换为Grayscale8
            *pobjImageBuffer = pobjImageBuffer->convertToFormat(QImage::Format_Grayscale8);
        }
    }

    * pobjImageBuffer = pobjImageBuffer->mirrored(false, true);
    cv::flip(matImage, matImage, 0);
    pf->PushBackToShowImageDeque(pobjImageBuffer);
    pf->PushBackToMatDeque(matImage);
    pf->m_nFrameCount++;
    pf->m_objFps.IncreaseFrameNum();
    emit pf->SigGetSuccess4();
}

void AcquisitionThread::PushBackToMatDeque(cv::Mat image){
    QMutexLocker locker(&m_objMatMutex);
    m_objMatBufferDeque.push_back(image);

    return;
}

void AcquisitionThread::PushBackToShowImageDeque(std::shared_ptr<QImage> pobjImage){
    QMutexLocker locker(&m_objDequeMutex);
    m_objShowImageDeque.push_back(pobjImage);

    return;
}

std::shared_ptr<QImage> AcquisitionThread::PopFrontFromShowImageDeque(){
    QMutexLocker locker(&m_objDequeMutex);

    if (m_objShowImageDeque.empty())
    {
        return NULL;
    }

    std::shared_ptr<QImage> pobjImage = m_objShowImageDeque.front();
    m_objShowImageDeque.pop_front();

    return pobjImage;
}

cv::Mat AcquisitionThread::PopFrontFromMatDeque(){
    QMutexLocker locker(&m_objMatMutex);

    if (m_objMatBufferDeque.empty())
    {
        return cv::Mat();
    }

    cv::Mat pobjImage = m_objMatBufferDeque.front();
    m_objMatBufferDeque.pop_front();

    return pobjImage;
}

std::shared_ptr<QImage> AcquisitionThread::GetFrontShowImageDeque(){
    QMutexLocker locker(&m_objDequeMutex);

    if (m_objShowImageDeque.empty())
    {
        return NULL;
    }

    std::shared_ptr<QImage> pobjImage = m_objShowImageDeque.front();

    return pobjImage;
}

void AcquisitionThread::ClearDeque(){
    QMutexLocker locker(&m_objDequeMutex);

    m_objMatBufferDeque.clear();
    m_objShowImageDeque.clear();

    return;
}


double AcquisitionThread::GetImageAcqFps()
{
    m_objFps.UpdateFps();
    double dAcqFrameRate = m_objFps.GetFps();

    return dAcqFrameRate;
}
