#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pstBaseInfo(NULL)
    , m_ui32DeviceNum(0)
    ,is_Start(false)
    ,m_nOperateID(-1)
    ,deviceHandles(NULL)
    ,m_pstCam(NULL)
    ,m_pCamsFps(NULL)
    ,m_CamAcq1(NULL)
    ,m_CamAcq2(NULL)
    ,m_CamAcq3(NULL)
    ,m_CamAcq4(NULL)
    ,m_ShowFrameRateTimer1(NULL)
    ,m_ShowFrameRateTimer2(NULL)
    ,m_ShowFrameRateTimer3(NULL)
    ,m_ShowFrameRateTimer4(NULL)
    ,SampleTimer1(NULL)
    ,SampleTimer2(NULL)
    ,m_Control(NULL)
    ,matcher(
          Config::GetClassesListPath(),
          Config::GetConfusionMatrixPath(),
          Config::GetDistanceThreshold(),
          Config::GetDistanceDiffThreshold()
          )
    ,testDataset("data/test_dataset.csv")
{
    ui->setupUi(this);


    m_ShowFrameRateTimer1 = new QTimer(this);
    m_ShowFrameRateTimer2 = new QTimer(this);
    m_ShowFrameRateTimer3 = new QTimer(this);
    m_ShowFrameRateTimer4 = new QTimer(this);
    SampleTimer1 = new QTimer(this);
    SampleTimer2 = new QTimer(this);
    SampleTimer3 = new QTimer(this);
    SampleTimer4 = new QTimer(this);
    // emptyImage = cv::imread("images/1_1.jpg");
    /*********************************************************************************/
    // 连接所有定时器信号到同一个槽
    connect(m_ShowFrameRateTimer1, &QTimer::timeout, this, [=](){ slotShowFrameRate(0); });
    connect(m_ShowFrameRateTimer2, &QTimer::timeout, this, [=](){ slotShowFrameRate(1); });
    connect(m_ShowFrameRateTimer3, &QTimer::timeout, this, [=](){ slotShowFrameRate(2); });
    connect(m_ShowFrameRateTimer4, &QTimer::timeout, this, [=](){ slotShowFrameRate(3); });
    /*********************************************************************************/

    connect(SampleTimer1,SIGNAL(timeout()), this, SLOT(slotSaveFirstCamImage()));
    connect(SampleTimer2,SIGNAL(timeout()), this, SLOT(slotSaveSecondCamImage()));
    connect(SampleTimer3,SIGNAL(timeout()),this,SLOT(slotSaveThirdCamImage()));
    connect(SampleTimer4,SIGNAL(timeout()),this,SLOT(slotSaveForthCamImage()));
    // 使用信号槽连接 forward 和 backward 图像捕获信号
    connect(this, &MainWindow::firstCamImageCaptured, this, &MainWindow::FirstCamImagesCapturedAndReady);
    connect(this, &MainWindow::secondCamImageCaptured, this, &MainWindow::SecondCamImagesCapturedAndReady);
    connect(this, &MainWindow::thirdCamImageCaptured, this, &MainWindow::ThirdCamImagesCapturedAndReady);
    connect(this, &MainWindow::forthCamImageCaptured, this, &MainWindow::ForthCamImagesCapturedAndReady);

    /*********************************************************************************/
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXInitLib();
    if (emStatus != GX_STATUS_SUCCESS)
    {
        ShowErrorString(emStatus);
    }
    /*********************************************************************************/
    setupModelComboBox();
    /*********************************************************************************/
    connect(ui->resultLabel_1, &ClickableLabel::clicked, this, &MainWindow::showZoomedImage1);
    connect(ui->resultLabel_2, &ClickableLabel::clicked, this, &MainWindow::showZoomedImage2);
    /*********************************************************************************/
    QFont font("Courier");
    ui->Log_textBrowser->setFont(font);
}

MainWindow::~MainWindow()
{
    GX_STATUS emStatus = GX_STATUS_ERROR;

    //释放内存
    if(m_pstBaseInfo != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstBaseInfo);
    }
    if(deviceHandles != NULL)
    {
        RELEASE_ALLOC_ARR(deviceHandles);
    }
    if (m_pCamsFps != NULL)
    {
        RELEASE_ALLOC_ARR(m_pCamsFps);
    }

    if(m_pstCam != NULL)
    {
        for(uint32_t i = 0; i < m_ui32DeviceNum; i++)
        {
            if(m_pstCam[i].pImageBuffer != NULL)
            {
                RELEASE_ALLOC_ARR(m_pstCam[i].pImageBuffer);
            }
            if (m_pstCam[i].pRawBuffer != NULL)
            {
                RELEASE_ALLOC_ARR(m_pstCam[i].pRawBuffer);
            }
        }
        RELEASE_ALLOC_ARR(m_pstCam);
    }

    //关闭库
    emStatus = GXCloseLib();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 获取当前时间
    QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    // 设置文件名，使用当前时间戳
    QString fileName = "log_" + currentTime + ".txt";

    // 保存路径（当前应用程序目录下）
    QString filePath = QDir::currentPath() + "/exe_logs/" + fileName;

    // 获取QTextBrowser的内容
    QString textContent = ui->Log_textBrowser->toPlainText();

    // 打开文件进行写入
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << textContent;
        file.close();
        ui->Log_textBrowser->append("Log saved to: " + filePath);  // 可选：在界面显示保存路径
    } else {
        ui->Log_textBrowser->append("Failed to save log!");  // 可选：在界面显示保存失败
    }

    // 确保关闭事件正常进行
    event->accept();
}

std::ostream& operator<<(std::ostream& os, const PositionType& position) {
    switch (position) {
    case PositionType::NONE: os << "NONE"; break;
    case PositionType::FINE: os << "FINE"; break;
    case PositionType::LEFT: os << "LEFT"; break;
    case PositionType::RIGHT: os << "RIGHT"; break;
    }
    return os;
}


//更新当前UI
void MainWindow::UpdateUI(){

}

//获取相机设备信息,并申请空间
void MainWindow::UpdateDeviceList(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    RELEASE_ALLOC_ARR(m_pstBaseInfo);
    emStatus = GXUpdateDeviceList(&m_ui32DeviceNum, ENUMRATE_TIME_OUT);
    GX_VERIFY(emStatus);
    if(m_ui32DeviceNum > 0)
    {
        try
        {
            m_pstBaseInfo = new GX_DEVICE_BASE_INFO[m_ui32DeviceNum];
            deviceHandles = new GX_DEV_HANDLE[m_ui32DeviceNum];
            m_pstCam = new CAMER_INFO[m_ui32DeviceNum];
            m_pCamsFps = new CFps[m_ui32DeviceNum];
        }
        catch (std::bad_alloc &e)
        {
            QMessageBox::about(NULL, "Allocate memory error", "Cannot allocate memory, please exit this app!");
            RELEASE_ALLOC_MEM(m_pstBaseInfo);
            return;
        }

        for (uint32_t i=0; i<m_ui32DeviceNum; i++)
        {
            deviceHandles[i]  = NULL;

            m_pstCam[i].bIsColorFilter = false;
            m_pstCam[i].bIsOpen        = FALSE;
            m_pstCam[i].bIsSnap        = FALSE;
            m_pstCam[i].pBmpInfo       = NULL;
            m_pstCam[i].pRawBuffer     = NULL;
            m_pstCam[i].pImageBuffer   = NULL;
            m_pstCam[i].fFps           = 0.0;
            m_pstCam[i].nBayerLayout   = 0;
            m_pstCam[i].nImageHeight   = 0;
            m_pstCam[i].nImageWidth    = 0;
            m_pstCam[i].nPayLoadSise   = 0;
            memset(m_pstCam[i].chBmpBuf,0,sizeof(m_pstCam[i].chBmpBuf));
        }
        size_t nSize = m_ui32DeviceNum * sizeof(GX_DEVICE_BASE_INFO);

        emStatus = GXGetAllDeviceBaseInfo(m_pstBaseInfo, &nSize);
        if (emStatus != GX_STATUS_SUCCESS)
        {
            RELEASE_ALLOC_ARR(m_pstBaseInfo);
            ShowErrorString(emStatus);
            m_ui32DeviceNum = 0;
            return;
        }
    }

    return;
}

//更新相机设备列表 显示在文本框中
void MainWindow::on_UpdateDeviceList_clicked()
{
    QString szDeviceDisplayName;
    ui->DeviceList->clear();
    UpdateDeviceList();
    if (m_ui32DeviceNum == 0)
    {
        UpdateUI();
        return;
    }
    for (uint32_t i = 0; i < m_ui32DeviceNum; i++)
    {
        QString szDeviceDisplayName = QString("%1").arg(m_pstBaseInfo[i].szDisplayName);
        ui->DeviceList->addItem(QString(szDeviceDisplayName));
    }
    ui->DeviceList->setCurrentIndex(0);
    UpdateUI();

    return;
}

//输入ip地址后，点击按钮后和PLC连接
void MainWindow::on_PlcConnect_clicked()
{
    QString PLCIp = ui->PLC_ip_get_lineEdit->text();
    QByteArray byteArray = PLCIp.toUtf8();
    char ipArray[16];
    strncpy(ipArray, byteArray.data(), sizeof(ipArray) - 1);
    ipArray[sizeof(ipArray) - 1] = '\0';
    BOOL bRet = Init_ETH_String(ipArray,GlobalVariable::GetInstance().nNetId,GlobalVariable::GetInstance().nIpPort);
    if(bRet){
        ui->Log_textBrowser->append("连接成功");
    }else{
        ui->Log_textBrowser->append("连接失败");
    }
}

//PLC断开连接
void MainWindow::on_PlcDisConnect_clicked()
{
    BOOL  bRet = Exit_ETH (GlobalVariable::GetInstance().nNetId);
    if(bRet){
        ui->Log_textBrowser->append("断开连接成功");
    }else{
        ui->Log_textBrowser->append("断开连接失败");
    }
}

//按照需要打开的相机数目打开设备
void MainWindow::OpenDevice(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    int64_t   nValue   = 0;
    char      szIndex[10]    = {0};
    bool      bIsImplemented = false;
    char      emptyString[] = "";

    // 定义并初始化设备打开参数
    GX_OPEN_PARAM stOpenParam;
    stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
    stOpenParam.openMode   = GX_OPEN_INDEX;
    stOpenParam.pszContent = emptyString;

    m_nOperateID = ui->DeviceList->currentIndex();
    _itoa(m_nOperateID + 1, szIndex, 10);

    if (deviceHandles[m_nOperateID] != NULL)
    {
        emStatus = GXCloseDevice(deviceHandles[m_nOperateID]);
        GX_VERIFY(emStatus);
        deviceHandles[m_nOperateID] = NULL;
    }

    stOpenParam.pszContent = szIndex;
    emStatus = GXOpenDevice(&stOpenParam, &deviceHandles[m_nOperateID]);

    // 根据当前网络环境设置相机的流通道包长值，以提高网络相机的采集性能.
    {
        bool	 bImplementPacketSize = false;
        uint32_t unPacketSize		  = 0;

        // 判断设备是否支持流通道数据包功能
        emStatus = GXIsImplemented(deviceHandles[m_nOperateID], GX_INT_GEV_PACKETSIZE, &bImplementPacketSize);
        GX_VERIFY(emStatus);

        if (bImplementPacketSize)
        {
            // 获取当前网络环境的最优包长值
            emStatus = GXGetOptimalPacketSize(deviceHandles[m_nOperateID],&unPacketSize);
            GX_VERIFY(emStatus);

            // 将最优包长值设置为当前设备的流通道包长值
            emStatus = GXSetInt(deviceHandles[m_nOperateID], GX_INT_GEV_PACKETSIZE, unPacketSize);
            GX_VERIFY(emStatus);
        }
    }
    m_pstCam[m_nOperateID].bIsOpen = TRUE;

    //获取相机Bayer转换类型及是否支持输出彩色图像
    emStatus = GXIsImplemented(deviceHandles[m_nOperateID], GX_ENUM_PIXEL_COLOR_FILTER, &bIsImplemented);
    GX_VERIFY(emStatus);
    m_pstCam[m_nOperateID].bIsColorFilter = bIsImplemented;
    if (bIsImplemented)
    {
        emStatus = GXGetEnum(deviceHandles[m_nOperateID], GX_ENUM_PIXEL_COLOR_FILTER, &nValue);
        GX_VERIFY(emStatus);
        m_pstCam[m_nOperateID].nBayerLayout = nValue;
    }

    // 获取图像宽
    emStatus = GXGetInt(deviceHandles[m_nOperateID], GX_INT_WIDTH, &nValue);
    GX_VERIFY(emStatus);
    m_pstCam[m_nOperateID].nImageWidth = nValue;

    // 获取图像高
    emStatus = GXGetInt(deviceHandles[m_nOperateID], GX_INT_HEIGHT, &nValue);
    GX_VERIFY(emStatus);
    m_pstCam[m_nOperateID].nImageHeight = nValue;


    // 获取原始图像大小
    emStatus = GXGetInt(deviceHandles[m_nOperateID], GX_INT_PAYLOAD_SIZE, &nValue);
    GX_VERIFY(emStatus);
    m_pstCam[m_nOperateID].nPayLoadSise = nValue;

    //设置采集模式连续采集
    emStatus = GXSetEnum(deviceHandles[m_nOperateID], GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY(emStatus);

    ui->Log_textBrowser->append("相机打开成功");
    return;
}

//
void MainWindow::CloseDevice(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    m_nOperateID = ui->DeviceList->currentIndex();


    if(m_pstCam[m_nOperateID].pImageBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pImageBuffer);
    }

    if (m_pstCam[m_nOperateID].pRawBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pRawBuffer);
    }


    emStatus = GXCloseDevice(deviceHandles[m_nOperateID]);
    GX_VERIFY(emStatus);

    deviceHandles[m_nOperateID] = NULL;
    m_pstCam[m_nOperateID].bIsOpen = false;

    UpdateUI();

    return;
}

//显示相机的基本信息
void MainWindow::ShowDeviceInfo(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    const int nStringLength = 128;
    size_t nSize = 0;

    char arrVendorNameString[nStringLength];
    nSize = sizeof(arrVendorNameString);
    m_nOperateID = ui->DeviceList->currentIndex();
    emStatus = GXGetString(deviceHandles[m_nOperateID], GX_STRING_DEVICE_VENDOR_NAME, arrVendorNameString, &nSize);
    GX_VERIFY(emStatus);

    char arrModelNameString[nStringLength];
    nSize = sizeof(arrModelNameString);
    emStatus = GXGetString(deviceHandles[m_nOperateID], GX_STRING_DEVICE_MODEL_NAME, arrModelNameString, &nSize);
    GX_VERIFY(emStatus);

    char arrSerialNumberString[nStringLength];
    nSize = sizeof(arrSerialNumberString);
    emStatus = GXGetString(deviceHandles[m_nOperateID], GX_STRING_DEVICE_SERIAL_NUMBER, arrSerialNumberString, &nSize);
    GX_VERIFY(emStatus);

    char arrDeviceVersionString[nStringLength];
    nSize = sizeof(arrDeviceVersionString);
    emStatus = GXGetString(deviceHandles[m_nOperateID], GX_STRING_DEVICE_VERSION, arrDeviceVersionString, &nSize);
    GX_VERIFY(emStatus);

    if(m_nOperateID==0){
        ui->VendorName_1->setText(QString("VendorName: %1").arg(QString(arrVendorNameString)));
        ui->ModelName_1->setText(QString("ModelName: %1").arg(QString(arrModelNameString)));
        ui->SerialNumber_1->setText(QString("SN: %1").arg(QString(arrSerialNumberString)));
        ui->DeviceVersion_1->setText(QString("DeviceVersion: %1").arg(QString(arrDeviceVersionString)));
    }
    if(m_nOperateID==1){
        ui->VendorName_2->setText(QString("VendorName: %1").arg(QString(arrVendorNameString)));
        ui->ModelName_2->setText(QString("ModelName: %1").arg(QString(arrModelNameString)));
        ui->SerialNumber_2->setText(QString("SN: %1").arg(QString(arrSerialNumberString)));
        ui->DeviceVersion_2->setText(QString("DeviceVersion: %1").arg(QString(arrDeviceVersionString)));
    }
    if(m_nOperateID==2){
        ui->VendorName_3->setText(QString("VendorName: %1").arg(QString(arrVendorNameString)));
        ui->ModelName_3->setText(QString("ModelName: %1").arg(QString(arrModelNameString)));
        ui->SerialNumber_3->setText(QString("SN: %1").arg(QString(arrSerialNumberString)));
        ui->DeviceVersion_3->setText(QString("DeviceVersion: %1").arg(QString(arrDeviceVersionString)));
    }
    if(m_nOperateID==3){
        ui->VendorName_4->setText(QString("VendorName: %1").arg(QString(arrVendorNameString)));
        ui->ModelName_4->setText(QString("ModelName: %1").arg(QString(arrModelNameString)));
        ui->SerialNumber_4->setText(QString("SN: %1").arg(QString(arrSerialNumberString)));
        ui->DeviceVersion_4->setText(QString("DeviceVersion: %1").arg(QString(arrDeviceVersionString)));
    }
    return;
}

//获得相机基本参数 这里为了简单，直接拿第一个相机作为总体
void MainWindow::GetDeviceInitParam(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    emStatus = InitComboBox(deviceHandles[0], ui->PixelFormat, GX_ENUM_PIXEL_FORMAT);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CloseDevice();
        GX_VERIFY(emStatus);
    }

    emStatus = InitComboBox(deviceHandles[0], ui->TriggerMode, GX_ENUM_TRIGGER_MODE);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CloseDevice();
        GX_VERIFY(emStatus);
    }

    if (ui->TriggerMode->itemData(ui->TriggerMode->currentIndex()).value<int64_t>() == GX_TRIGGER_MODE_ON)
    {
        m_bTriggerModeOn = true;
    }
    else
    {
        m_bTriggerModeOn = false;
    }

    emStatus = InitComboBox(deviceHandles[0], ui->TriggerSource, GX_ENUM_TRIGGER_SOURCE);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CloseDevice();
        GX_VERIFY(emStatus);
    }

    if (ui->TriggerSource->itemData(ui->TriggerSource->currentIndex()).value<int64_t>() == GX_TRIGGER_SOURCE_SOFTWARE)
    {
        m_bSoftTriggerOn = true;
    }
    else
    {
        m_bSoftTriggerOn = false;
    }

    return;
}

//点击打开相机按钮相应
void MainWindow::on_OpenDevice_clicked()
{
    OpenDevice();
    SetUpAcquisitionThread();
    ShowDeviceInfo();
    GetDeviceInitParam();
    UpdateUI();

    return;
}

//
void MainWindow::on_TriggerMode_activated(int index)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    for(uint32_t i = 0;i<m_ui32DeviceNum;i++){
        emStatus = GXSetEnum(deviceHandles[i], GX_ENUM_TRIGGER_MODE, ui->TriggerMode->itemData(index).value<int64_t>());
        GX_VERIFY(emStatus);
    }

    if (ui->TriggerMode->itemData(index).value<int64_t>() == GX_TRIGGER_MODE_ON)
    {
        m_bTriggerModeOn = true;
    }
    else
    {
        m_bTriggerModeOn = false;
    }

    if (ui->TriggerSource->itemData(ui->TriggerSource->currentIndex()).value<int64_t>() == GX_TRIGGER_SOURCE_SOFTWARE)
    {
        m_bSoftTriggerOn = true;
    }
    else
    {
        m_bSoftTriggerOn = false;
    }

    UpdateUI();

    return;
}

//
void MainWindow::on_TriggerSource_activated(int index)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    for(uint32_t i = 0;i<m_ui32DeviceNum;i++){
        emStatus = GXSetEnum(deviceHandles[i], GX_ENUM_TRIGGER_SOURCE,  ui->TriggerSource->itemData(index).value<int64_t>());
        GX_VERIFY(emStatus);
    }
    if (ui->TriggerSource->itemData(index).value<int64_t>() == GX_TRIGGER_SOURCE_SOFTWARE)
    {
        m_bSoftTriggerOn = true;
    }
    else
    {
        m_bSoftTriggerOn = false;
    }

    UpdateUI();

    return;
}

//
void MainWindow::on_TriggerSoftWare_clicked()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    for(uint32_t i = 0;i<m_ui32DeviceNum;i++){
        emStatus = GXSendCommand(deviceHandles[i], GX_COMMAND_TRIGGER_SOFTWARE);
        GX_VERIFY(emStatus);
    }
    return;
}


void MainWindow::on_CloseDevice_clicked()
{
    // ClearUI();
    // DestroyDialogs();
    CloseDevice();
    UpdateUI();

    return;
}

void MainWindow::on_StartAcquisition_clicked()
{
    GX_STATUS emStatus       = GX_STATUS_ERROR;
    bool      bIsColorFilter = m_pstCam[m_nOperateID].bIsColorFilter;
    m_nOperateID = ui->DeviceList->currentIndex();

    // 清空Buffer
    if(m_pstCam[m_nOperateID].pImageBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pImageBuffer);
    }

    if (m_pstCam[m_nOperateID].pRawBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pRawBuffer);
    }

    // 申请资源
    m_pstCam[m_nOperateID].pRawBuffer = new BYTE[(size_t)(m_pstCam[m_nOperateID].nPayLoadSise)];
    if(m_pstCam[m_nOperateID].pRawBuffer == NULL)
    {
        ui->Log_textBrowser->append("申请资源失败!");
        return;
    }

    // 如果是彩色相机，为彩色相机申请资源
    if(bIsColorFilter)
    {
        m_pstCam[m_nOperateID].pImageBuffer = new BYTE[(size_t)(m_pstCam[m_nOperateID].nImageWidth * m_pstCam[m_nOperateID].nImageHeight * 3)];
        if (m_pstCam[m_nOperateID].pImageBuffer == NULL)
        {
            ui->Log_textBrowser->append("申请资源失败!");
            RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pRawBuffer);
            return;
        }
    }
    // 如果是黑白相机， 为黑白相机申请资源
    else
    {
        m_pstCam[m_nOperateID].pImageBuffer = new BYTE[(size_t)(m_pstCam[m_nOperateID].nImageWidth * m_pstCam[m_nOperateID].nImageHeight)];
        if (m_pstCam[m_nOperateID].pImageBuffer == NULL)
        {
            ui->Log_textBrowser->append("申请资源失败!");
            RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pRawBuffer);
            return;
        }
    }


    const int nFrameRateTimerInterval = 500;

    switch (m_nOperateID) {
    case 0:
        m_CamAcq1->GetParam(deviceHandles,m_pstCam,m_nOperateID);
        m_CamAcq1->start();
        m_ShowFrameRateTimer1->start(nFrameRateTimerInterval);
        break;
    case 1:
        m_CamAcq2->GetParam(deviceHandles,m_pstCam,m_nOperateID);
        m_CamAcq2->start();
        m_ShowFrameRateTimer2->start(nFrameRateTimerInterval);
        break;
    case 2:
        m_CamAcq3->GetParam(deviceHandles,m_pstCam,m_nOperateID);
        m_CamAcq3->start();
        m_ShowFrameRateTimer3->start(nFrameRateTimerInterval);
        break;
    case 3:
        m_CamAcq4->GetParam(deviceHandles,m_pstCam,m_nOperateID);
        m_CamAcq4->start();
        m_ShowFrameRateTimer4->start(nFrameRateTimerInterval);
        break;
    default:
        break;
    }



    m_pstCam[m_nOperateID].bIsSnap = TRUE;

    UpdateUI();
}

void MainWindow::SetUpAcquisitionThread(){
    m_nOperateID = ui->DeviceList->currentIndex();
    try
    {
        switch (m_nOperateID) {
        case 0:
            m_CamAcq1 = new AcquisitionThread();
            connect(m_CamAcq1,&AcquisitionThread::SigGetSuccess1,this,&MainWindow::slotShowImage1,Qt::QueuedConnection);
            break;
        case 1:
            m_CamAcq2 = new AcquisitionThread();
            connect(m_CamAcq2,&AcquisitionThread::SigGetSuccess2,this,&MainWindow::slotShowImage2,Qt::QueuedConnection);
            break;
        case 2:
            m_CamAcq3 = new AcquisitionThread();
            connect(m_CamAcq3,&AcquisitionThread::SigGetSuccess3,this,&MainWindow::slotShowImage3,Qt::QueuedConnection);
            break;
        case 3:
            m_CamAcq4 = new AcquisitionThread();
            connect(m_CamAcq4,&AcquisitionThread::SigGetSuccess4,this,&MainWindow::slotShowImage4,Qt::QueuedConnection);
            break;
        default:
            break;
        }
    }
    catch (std::bad_alloc &e)
    {
        QMessageBox::about(NULL, "Allocate memory error", "Cannot allocate memory, please exit this app!");
        RELEASE_ALLOC_MEM(m_CamAcq1);
        RELEASE_ALLOC_MEM(m_CamAcq2);
        RELEASE_ALLOC_MEM(m_CamAcq3);
        RELEASE_ALLOC_MEM(m_CamAcq4);
        return;
    }
    return;
}

void MainWindow::on_StopAcquisition_clicked()
{
    GX_STATUS emStatus = GX_STATUS_ERROR;
    m_nOperateID = ui->DeviceList->currentIndex();

    //停止采集
    emStatus = GXSendCommand(deviceHandles[m_nOperateID], GX_COMMAND_ACQUISITION_STOP);
    emStatus = GXUnregisterCaptureCallback(deviceHandles[m_nOperateID]);
    GX_VERIFY(emStatus);

    //释放资源
    if(m_pstCam[m_nOperateID].pImageBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pImageBuffer);
    }

    if (m_pstCam[m_nOperateID].pRawBuffer != NULL)
    {
        RELEASE_ALLOC_ARR(m_pstCam[m_nOperateID].pRawBuffer);
    }

    m_pCamsFps[m_nOperateID].Reset();
    m_pstCam[m_nOperateID].bIsSnap = FALSE;
    switch (m_nOperateID) {
    case 0:
        m_CamAcq1->quit();
        m_CamAcq1->wait();
        m_pCamsFps[m_nOperateID].Reset();
        m_ShowFrameRateTimer1->stop();
        break;
    case 1:
        m_CamAcq2->quit();
        m_CamAcq2->wait();
        m_pCamsFps[m_nOperateID].Reset();
        m_ShowFrameRateTimer2->stop();
        break;
    case 2:
        m_CamAcq3->quit();
        m_CamAcq3->wait();
        m_pCamsFps[m_nOperateID].Reset();
        m_ShowFrameRateTimer3->stop();
        break;
    case 3:
        m_CamAcq4->quit();
        m_CamAcq4->wait();
        m_pCamsFps[m_nOperateID].Reset();
        m_ShowFrameRateTimer4->stop();
        break;
    default:
        break;
    }
}

void MainWindow::slotShowImage(int camIndex, AcquisitionThread* camAcq, QLabel* label,
                                    std::unordered_map<int, std::deque<cv::Mat>>& photoQueue, int& num, bool& isNew,
                                    const char* signalName, bool& isCaptured){
    std::shared_ptr<QImage> m_ImageForShow = camAcq->PopFrontFromShowImageDeque();
    QImage objImgScaled = m_ImageForShow->scaled(label->width(), label->height(),
                                                 Qt::IgnoreAspectRatio, Qt::FastTransformation);
    label->setPixmap(QPixmap::fromImage(objImgScaled));

    // Capture the image and handle FPS
    cv::Mat img = camAcq->PopFrontFromMatDeque();
    m_pCamsFps[camIndex].IncreaseFrameNum();

    // Check if new image and store it
    if (isNew) {
        // Dynamically invoke the signal method
        QMetaObject::invokeMethod(this, signalName);
        photoQueue[num].push_back(img);
        isCaptured = true;
    }
}

void MainWindow::slotShowImage1(){
    slotShowImage(0, m_CamAcq1, ui->ImageLabel_1, GlobalVariable::GetInstance().photoQueues1,
                  GlobalVariable::GetInstance().Num1, GlobalVariable::GetInstance().isNew1,
                  "firstCamImageCaptured", is1);
}

void MainWindow::slotShowImage2(){
    slotShowImage(1, m_CamAcq2, ui->ImageLabel_2, GlobalVariable::GetInstance().photoQueues2,
                  GlobalVariable::GetInstance().Num2, GlobalVariable::GetInstance().isNew2,
                  "secondCamImageCaptured", is2);
}

void MainWindow::slotShowImage3(){
    slotShowImage(2, m_CamAcq3, ui->ImageLabel_3, GlobalVariable::GetInstance().photoQueues3,
                  GlobalVariable::GetInstance().Num3, GlobalVariable::GetInstance().isNew3,
                  "thirdCamImageCaptured", is3);
}

void MainWindow::slotShowImage4(){
    slotShowImage(3, m_CamAcq4, ui->ImageLabel_4, GlobalVariable::GetInstance().photoQueues4,
                  GlobalVariable::GetInstance().Num4, GlobalVariable::GetInstance().isNew4,
                  "forthCamImageCaptured", is4);
}

double MainWindow::GetImageShowFps(int index)
{
    double dImgShowFrameRate = 0.0;
    m_pCamsFps[index].UpdateFps();
    dImgShowFrameRate = m_pCamsFps[index].GetFps();

    return dImgShowFrameRate;
}

void MainWindow::slotShowFrameRate(int index)
{
    double dImgShowFrameRate = GetImageShowFps(index);
    double dAcqFrameRate = 0.0;
    QString acqText;
    QString dispText;

    switch (index)
    {
    case 0:
        acqText = QString("Frame NUM: %1   Acq. FPS: %2")
                      .arg(m_CamAcq1->m_nFrameCount)
                      .arg(dAcqFrameRate, 0, 'f', 1);
        dispText = QString("Disp.FPS: %1").arg(dImgShowFrameRate, 0, 'f', 1);
        ui->AcqFrameRateLabel_1->setText(acqText);
        ui->ShowFrameRateLabel_1->setText(dispText);
        break;
    case 1:
        acqText = QString("Frame NUM: %1   Acq. FPS: %2")
                      .arg(m_CamAcq2->m_nFrameCount)
                      .arg(dAcqFrameRate, 0, 'f', 1);
        dispText = QString("Disp.FPS: %1").arg(dImgShowFrameRate, 0, 'f', 1);
        ui->AcqFrameRateLabel_2->setText(acqText);
        ui->ShowFrameRateLabel_2->setText(dispText);
        break;
    case 2:
        acqText = QString("Frame NUM: %1   Acq. FPS: %2")
                      .arg(m_CamAcq3->m_nFrameCount)
                      .arg(dAcqFrameRate, 0, 'f', 1);
        dispText = QString("Disp.FPS: %1").arg(dImgShowFrameRate, 0, 'f', 1);
        ui->AcqFrameRateLabel_3->setText(acqText);
        ui->ShowFrameRateLabel_3->setText(dispText);
        break;
    case 3:
        acqText = QString("Frame NUM: %1   Acq. FPS: %2")
                      .arg(m_CamAcq4->m_nFrameCount)
                      .arg(dAcqFrameRate, 0, 'f', 1);
        dispText = QString("Disp.FPS: %1").arg(dImgShowFrameRate, 0, 'f', 1);
        ui->AcqFrameRateLabel_4->setText(acqText);
        ui->ShowFrameRateLabel_4->setText(dispText);
        break;
    default:
        break;
    }
}

void MainWindow::on_StartVerify_clicked()
{
    m_Control = new ControlThread();
    is_Start = true;
    connect(m_Control, &ControlThread::logMessage, this, &MainWindow::onLogMessage);

    connect(m_Control,&ControlThread::StartFirstCamTask,this,&MainWindow::slotStartFirstCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::EndFirstCamTask,this,&MainWindow::slotEndFirstCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::StartSecondCamTask,this,&MainWindow::slotStartSecondCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::EndSecondCamTask,this,&MainWindow::slotEndSecondCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::StartThirdCamTask,this,&MainWindow::slotStartThirdCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::EndThirdCamTask,this,&MainWindow::slotEndThirdCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::StartForthCamTask,this,&MainWindow::slotStartForthCamTask,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::EndForthCamTask,this,&MainWindow::slotEndForthCamTask,Qt::QueuedConnection);

    connect(m_Control,&ControlThread::GetResult,this,&MainWindow::slotUpdateCombox1,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::GetResult,this,&MainWindow::slotUpdateCombox2,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::GetResult,this,&MainWindow::slotUpdateCombox3,Qt::QueuedConnection);
    connect(m_Control,&ControlThread::GetResult,this,&MainWindow::slotUpdateCombox4,Qt::QueuedConnection);
    m_Control->StartVerify();
    m_Control->start();
}

void MainWindow::on_StopVerify_clicked()
{
    m_Control->StopVerify();
}

void MainWindow::slotSaveFirstCamImage(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSendCommand(deviceHandles[0], GX_COMMAND_TRIGGER_SOFTWARE);
    GX_VERIFY(emStatus);
}

void MainWindow::slotSaveSecondCamImage(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSendCommand(deviceHandles[1], GX_COMMAND_TRIGGER_SOFTWARE);
    GX_VERIFY(emStatus);
}

void MainWindow::slotSaveThirdCamImage(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSendCommand(deviceHandles[2], GX_COMMAND_TRIGGER_SOFTWARE);
    GX_VERIFY(emStatus);
}

void MainWindow::slotSaveForthCamImage(){
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSendCommand(deviceHandles[3], GX_COMMAND_TRIGGER_SOFTWARE);
    GX_VERIFY(emStatus);
}

void MainWindow::onLogMessage(const QString &message){
    ui->Log_textBrowser->append(message);
}

void MainWindow::slotStartFirstCamTask(){
    SampleTimer1->start(SampleInterval);
    if(!detectStreams[0]->StartTask()){
        ui->Log_textBrowser->append("Start task failed");
    }
}

void MainWindow::slotEndFirstCamTask(){
    detectStreams[0]->EndTask();
    SampleTimer1->stop();
}

void MainWindow::slotStartSecondCamTask(){
    SampleTimer2->start(SampleInterval);
    if(!detectStreams[1]->StartTask()){
        ui->Log_textBrowser->append("Start task failed");
    }
}

void MainWindow::slotEndSecondCamTask(){
    detectStreams[1]->EndTask();
    SampleTimer2->stop();
}

void MainWindow::slotStartThirdCamTask(){
    SampleTimer3->start(SampleInterval);
    if(!detectStreams[2]->StartTask()){
        ui->Log_textBrowser->append("Start task failed");
    }
}

void MainWindow::slotEndThirdCamTask(){
    detectStreams[2]->EndTask();
    SampleTimer3->stop();
}

void MainWindow::slotStartForthCamTask(){
    SampleTimer4->start(SampleInterval);
    if(!detectStreams[3]->StartTask()){
        ui->Log_textBrowser->append("Start task failed");
    }
}

void MainWindow::slotEndForthCamTask(){
    detectStreams[3]->EndTask();
    SampleTimer4->stop();
}

void MainWindow::FirstCamImagesCapturedAndReady() {
    if (is1&&!GlobalVariable::GetInstance().photoQueues1[GlobalVariable::GetInstance().Num1].empty()) {
        // 检查 forward 和 backward 图片都已经准备好
        cv::Mat sectionImageForward =  GlobalVariable::GetInstance().photoQueues1[GlobalVariable::GetInstance().Num1].front();
        GlobalVariable::GetInstance().photoQueues1[GlobalVariable::GetInstance().Num1].pop_front();
        cv::Mat sectionImageBackward = emptyImage;
        detectStreams[0]->AddImage(sectionImageForward, sectionImageBackward);
        is1 = false;
    }
}

void MainWindow::SecondCamImagesCapturedAndReady() {
    if (is2&&!GlobalVariable::GetInstance().photoQueues2[GlobalVariable::GetInstance().Num2].empty()) {
        // 检查 forward 和 backward 图片都已经准备好
        cv::Mat sectionImageForward =  GlobalVariable::GetInstance().photoQueues2[GlobalVariable::GetInstance().Num2].front();
        GlobalVariable::GetInstance().photoQueues2[GlobalVariable::GetInstance().Num2].pop_front();
        cv::Mat sectionImageBackward = emptyImage;
        detectStreams[1]->AddImage(sectionImageForward, sectionImageBackward);
        is2 = false;
    }
}

void MainWindow::ThirdCamImagesCapturedAndReady() {
    if (is3&&!GlobalVariable::GetInstance().photoQueues3[GlobalVariable::GetInstance().Num3].empty()) {
        // 检查 forward 和 backward 图片都已经准备好
        cv::Mat sectionImageForward =  GlobalVariable::GetInstance().photoQueues3[GlobalVariable::GetInstance().Num3].front();
        GlobalVariable::GetInstance().photoQueues3[GlobalVariable::GetInstance().Num3].pop_front();
        cv::Mat sectionImageBackward = emptyImage;
        count1++;
        SaveImage(sectionImageForward,count1,3);
        detectStreams[2]->AddImage(sectionImageForward, sectionImageBackward);
        is3 = false;
    }
}

void MainWindow::ForthCamImagesCapturedAndReady() {
    if (is4&&!GlobalVariable::GetInstance().photoQueues4[GlobalVariable::GetInstance().Num4].empty()) {
        // 检查 forward 和 backward 图片都已经准备好
        cv::Mat sectionImageForward =  GlobalVariable::GetInstance().photoQueues4[GlobalVariable::GetInstance().Num4].front();
        GlobalVariable::GetInstance().photoQueues4[GlobalVariable::GetInstance().Num4].pop_front();
        cv::Mat sectionImageBackward = emptyImage;
        count2++;
        SaveImage(sectionImageForward,count2,4);
        detectStreams[3]->AddImage(sectionImageForward, sectionImageBackward);
        is4 = false;
    }
}

void MainWindow::updateCombox(int streamIndex, const std::unordered_map<int, int>& lengths, QComboBox* resultComboBox){
    int num = 1;
    std::vector<std::string> candidates;
    std::vector<MatchType> diff;
    QString resultMessage;
    QString directionMessage = QString("第%1组相机 ").arg(streamIndex+1)+(GlobalVariable::GetInstance().for_signal[0] == 0 ? "从左往右: " : "从右往左: ");
    while(true){
        std::optional<DetectSectionWork<TYPE>> res = detectStreams[streamIndex]->GetResult();
        if(res.has_value()){
            resultMessage = QString::fromStdString(res.value().output.result);
            candidates = testDataset.FindStampString(lengths.at(num), 10);
            MatchResult result = matcher.MatchString(res.value().output.result, candidates, diff);
            ui->Log_textBrowser->append(directionMessage);
            ui->Log_textBrowser->append("测量长度: " + QString::number(lengths.at(num)));
            if (result.matched) {
                QString diffResult;
                ui->Log_textBrowser->append("识别结果: " + resultMessage);
                ui->Log_textBrowser->append("匹配结果: " + QString::fromStdString(result.result));
                for (auto& d : diff) {
                    diffResult.append(d == MatchType::MATCH ? " " : "^");
                }
                ui->Log_textBrowser->append("整体区别: " + diffResult);
                resultComboBox->addItem(directionMessage + " " + resultMessage + " 匹配: " + QString::fromStdString(result.result));
            } else {
                ui->Log_textBrowser->append("识别结果: " + resultMessage);
                ui->Log_textBrowser->append("匹配结果: 无");
                resultComboBox->addItem(directionMessage + " " + resultMessage + " 匹配: 无");
            }

        }
        else{
            // resultMessage = "没检测出结果";
            // ui->Log_textBrowser->append(directionMessage + " " + resultMessage);
            // resultComboBox->addItem(directionMessage + " " + resultMessage);
            break;
        }
        num++;
    }
}

void MainWindow::slotUpdateCombox1(){
    GlobalVariable::GetInstance().for_signal[0] == 0 ?
        updateCombox(0, GlobalVariable::GetInstance().length1, ui->result_comboBox_1):
        updateCombox(0, GlobalVariable::GetInstance().length2, ui->result_comboBox_1);
}

void MainWindow::slotUpdateCombox2(){
    GlobalVariable::GetInstance().for_signal[0] == 0 ?
        updateCombox(1, GlobalVariable::GetInstance().length1, ui->result_comboBox_2):
        updateCombox(1, GlobalVariable::GetInstance().length2, ui->result_comboBox_2);
}

void MainWindow::slotUpdateCombox3(){
    count1 = 0;
    GlobalVariable::GetInstance().for_signal[0] == 0 ?
        updateCombox(2, GlobalVariable::GetInstance().length1, ui->result_comboBox_3):
        updateCombox(2, GlobalVariable::GetInstance().length2, ui->result_comboBox_3);
}

void MainWindow::slotUpdateCombox4(){
    count2 = 0;
    GlobalVariable::GetInstance().for_signal[0] == 0 ?
        updateCombox(3, GlobalVariable::GetInstance().length1, ui->result_comboBox_4):
        updateCombox(3, GlobalVariable::GetInstance().length2, ui->result_comboBox_4);
}

QImage MainWindow::MatToQImage(const cv::Mat& mat) {
    if (mat.empty()) { return QImage(); // 返回空图像
    } // 处理不同的图像格式
    if (mat.type() == CV_8UC3) { // 彩色图像
        return QImage(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_RGB888).rgbSwapped();
    }
    else if (mat.type() == CV_8UC1) { // 灰度图像
        return QImage(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_Grayscale8);
    } else { // 其他格式可根据需要进行处理
        return QImage();
    }
}

void MainWindow::on_result_comboBox_1_activated(int index)
{
    QString imgPath = QString("images/first/%1.jpg").arg(index);
    imgPath1 = imgPath;
    QImage image(imgPath);
    QImage ImgScaled = image.scaled(ui->resultLabel_1->width(), ui->resultLabel_1->height(),
                                               Qt::IgnoreAspectRatio, Qt::FastTransformation);
    ui->resultLabel_1->setPixmap(QPixmap::fromImage(ImgScaled));
}

void MainWindow::on_result_comboBox_2_activated(int index)
{
    QString imgPath = QString("images/second/%1.jpg").arg(index);
    imgPath1 = imgPath;
    QImage image(imgPath);
    QImage ImgScaled = image.scaled(ui->resultLabel_1->width(), ui->resultLabel_1->height(),
                                    Qt::IgnoreAspectRatio, Qt::FastTransformation);
    ui->resultLabel_1->setPixmap(QPixmap::fromImage(ImgScaled));
}

void MainWindow::on_result_comboBox_3_activated(int index)
{
    QString imgPath = QString("images/third/%1.jpg").arg(index);
    imgPath2 = imgPath;
    QImage image(imgPath);
    QImage ImgScaled = image.scaled(ui->resultLabel_2->width(), ui->resultLabel_2->height(),
                                    Qt::IgnoreAspectRatio, Qt::FastTransformation);
    ui->resultLabel_2->setPixmap(QPixmap::fromImage(ImgScaled));
}

void MainWindow::on_result_comboBox_4_activated(int index)
{
    QString imgPath = QString("images/forth/%1.jpg").arg(index);
    imgPath2 = imgPath;
    QImage image(imgPath);
    QImage ImgScaled = image.scaled(ui->resultLabel_2->width(), ui->resultLabel_2->height(),
                                    Qt::IgnoreAspectRatio, Qt::FastTransformation);
    ui->resultLabel_2->setPixmap(QPixmap::fromImage(ImgScaled));
}

void MainWindow::showZoomedImage1() {
    // 从 QLabel 中获取当前显示的图像
    QPixmap currentPixmap = ui->resultLabel_1->pixmap(Qt::ReturnByValue);
    if (!currentPixmap.isNull()) {
        QImage currentImage(imgPath1);

        // 创建 ImageViewer 窗口
        ImageViewer *viewer = new ImageViewer(currentImage, this);
        viewer->show();  // 显示放大的图像窗口
    }
}

void MainWindow::showZoomedImage2() {
    // 从 QLabel 中获取当前显示的图像
    QPixmap currentPixmap = ui->resultLabel_2->pixmap(Qt::ReturnByValue);
    if (!currentPixmap.isNull()) {
        QImage currentImage(imgPath2);

        // 创建 ImageViewer 窗口
        ImageViewer *viewer = new ImageViewer(currentImage, this);
        viewer->show();  // 显示放大的图像窗口
    }
}

void MainWindow::SaveImage(cv::Mat image, int imagecount, int dir){
    QString imagePath;
    switch(dir){
    case 1:
        imagePath = QString("images/first/%1_%2.jpg").arg(GlobalVariable::GetInstance().Num1).arg(imagecount);
        break;
    case 2:
        imagePath = QString("images/second/%1_%2.jpg").arg(GlobalVariable::GetInstance().Num2).arg(imagecount);
        break;
    case 3:
        imagePath = QString("images/third/%1_%2.jpg").arg(GlobalVariable::GetInstance().Num3).arg(imagecount);
        break;
    case 4:
        imagePath = QString("images/forth/%1_%2.jpg").arg(GlobalVariable::GetInstance().Num4).arg(imagecount);
        break;
    }
    cv::imwrite(imagePath.toStdString(), image);
}

void MainWindow::setupModelComboBox(){
    classesPath = Config::GetClassesListPath();
    reverseMapPath = Config::GetReverseMapPath();
    detectStreams.resize(sectionNum);
    // 获取data目录
    #ifndef USE_PADDLE
        QDir dataDir("data/yolo/");
    #else
        QDir dataDir("data/paddle/");
    #endif


    // 获取所有onnx文件
    QStringList onnxFiles = dataDir.entryList(QStringList() << "*.onnx", QDir::Files);

    int modelNum = onnxFiles.size();
    detectors.resize(modelNum);

    for(int i = 0; i < modelNum; i++){
        #ifndef USE_PADDLE
        modelPath = QString("data/yolo/"+onnxFiles[i]).toStdString();
        detectors[i] = new YoloOnnxDetector(Config::GetHorizontalMargin(),
                                            Config::GetMinSamples(),
                                            Config::GetConf(),
                                            Config::GetNmsThreshold(),
                                            Config::GetWidthThreshold(),
                                            Config::GetHeightThreshold(),
                                            Config::GetRevThreshold());
        if(detectors[i]->LoadModel(modelPath, isCuda)){
            ui->Log_textBrowser->append("model loaded");
        }
        if(detectors[i]->LoadClasses(classesPath)){
            ui->Log_textBrowser->append("classes loaded");
        }
        if(detectors[i]->LoadReverseMap(reverseMapPath)){
            ui->Log_textBrowser->append("reverse map loaded");
        }
        #else
        detectors[i] = new PaddleOnnxDetector(Config::GetHorizontalMargin(),
                                              Config::GetHeightThreshold(),
                                              Config::GetRevThreshold());
        if(detectors[i]->InitDet(Config::GetPaddleDetModelPath(), Config::GetPaddleDetBinaryThreshold(), Config::GetPaddleDetPolygonThreshold(), Config::GetPaddleDetUnclipRatio(), Config::GetPaddleDetMaxCandidates(), isCuda)){
            ui->Log_textBrowser->append("det loaded");
        }
        if(detectors[i]->InitCls(Config::GetPaddleClsModelPath(), isCuda)){
            ui->Log_textBrowser->append("cls loaded");
        }
        if(detectors[i]->InitRec(Config::GetPaddleRecModelPath(), Config::GetPaddleRecDictPath(), isCuda)){
            ui->Log_textBrowser->append("rec loaded");
        }
        #endif

    }

    // 将所有onnx文件添加到QComboBox中
    for (const QString &file : onnxFiles) {
        ui->GetModel_comboBox->addItem(file);  // 添加文件名
        ui->GetModel_comboBox_2->addItem(file);  // 添加文件名
        ui->GetModel_comboBox_3->addItem(file);  // 添加文件名
        ui->GetModel_comboBox_4->addItem(file);  // 添加文件名
    }

    // 设置默认选择
    ui->GetModel_comboBox->setCurrentIndex(0);
    ui->GetModel_comboBox_2->setCurrentIndex(0);
    ui->GetModel_comboBox_3->setCurrentIndex(0);
    ui->GetModel_comboBox_4->setCurrentIndex(0);
}

void MainWindow::on_GetModel_comboBox_activated(int index)
{
    if (detectStreams[0] != nullptr) {
        delete detectStreams[0];  // 删除原指针
        detectStreams[0] = nullptr;  // 防止悬空指针，设置为 nullptr
    }
    ui->Log_textBrowser->append("====第一个相机加载模型====");
    detectStreams[0] = new DetectStream<TYPE>(*(detectors[index]), 3, [&,index](cv::Mat img, TYPE output) {
        detectors[index]->DrawResult(img,output);
        {std::lock_guard<std::mutex> lock(ui_mutex);
        QString resultMessage = QString("result: %1 @ 第一个相机结果").arg(QString::fromStdString(output.result));
            ui->Log_textBrowser->append(resultMessage);}
        cv::imwrite("images/first/" + std::to_string(idx1++) + ".jpg", img);
        std::cout << "result: " << output.result << " @ " << 0 << std::endl;
    });
    if(!detectStreams[0]->Start()){
        ui->Log_textBrowser->append("Start failed");
    }
}

void MainWindow::on_GetModel_comboBox_2_activated(int index)
{
    if (detectStreams[1] != nullptr) {
        delete detectStreams[1];  // 删除原指针
        detectStreams[1] = nullptr;  // 防止悬空指针，设置为 nullptr
    }
    ui->Log_textBrowser->append("====第二个相机加载模型====");

    detectStreams[1] = new DetectStream<TYPE>(*(detectors[index]), 3, [&,index](cv::Mat img, TYPE output) {
        detectors[index]->DrawResult(img,output);
        {std::lock_guard<std::mutex> lock(ui_mutex);
        QString resultMessage = QString("result: %1 @ 第二个相机结果").arg(QString::fromStdString(output.result));
            ui->Log_textBrowser->append(resultMessage);}
        cv::imwrite("images/second/" + std::to_string(idx2++) + ".jpg", img);
        std::cout << "result: " << output.result << " @ " << 1 << std::endl;
    });
    if(!detectStreams[1]->Start()){
        ui->Log_textBrowser->append("Start failed");
    }
}

void MainWindow::on_GetModel_comboBox_3_activated(int index)
{
    if (detectStreams[2] != nullptr) {
        delete detectStreams[2];  // 删除原指针
        detectStreams[2] = nullptr;  // 防止悬空指针，设置为 nullptr
    }
    ui->Log_textBrowser->append("====第三个相机加载模型====");

    detectStreams[2] = new DetectStream<TYPE>(*(detectors[index]), 3, [&,index](cv::Mat img, TYPE output) {
        detectors[index]->DrawResult(img,output);
        {std::lock_guard<std::mutex> lock(ui_mutex);
        QString resultMessage = QString("result: %1 @ 第三个相机结果").arg(QString::fromStdString(output.result));
            ui->Log_textBrowser->append(resultMessage);}
        cv::imwrite("images/third/" + std::to_string(idx3++) + ".jpg", img);
        std::cout << "result: " << output.result << " @ " << 2 << std::endl;
    });
    if(!detectStreams[2]->Start()){
        ui->Log_textBrowser->append("Start failed");
    }
}

void MainWindow::on_GetModel_comboBox_4_activated(int index)
{
    if (detectStreams[3] != nullptr) {
        delete detectStreams[3];  // 删除原指针
        detectStreams[3] = nullptr;  // 防止悬空指针，设置为 nullptr
    }
    ui->Log_textBrowser->append("====第四个相机加载模型====");

    detectStreams[3] = new DetectStream<TYPE>(*(detectors[index]), 3, [&,index](cv::Mat img, TYPE output) {
        detectors[index]->DrawResult(img,output);
        {std::lock_guard<std::mutex> lock(ui_mutex);
        QString resultMessage = QString("result: %1 @ 第四个相机结果").arg(QString::fromStdString(output.result));
            ui->Log_textBrowser->append(resultMessage);}
        cv::imwrite("images/forth/" + std::to_string(idx4++) + ".jpg", img);
        std::cout << "result: " << output.result << " @ " << 3 << std::endl;
    });
    if(!detectStreams[3]->Start()){
        ui->Log_textBrowser->append("Start failed");
    }
}
