#include "ControlThread.h"

ControlThread::ControlThread(){

}

void ControlThread::run(){
    static int pre_first_signal = GlobalVariable::GetInstance().first_signal[0];
    static int pre_second_signal = GlobalVariable::GetInstance().second_signal[0];
    static int pre_third_signal = GlobalVariable::GetInstance().third_signal[0];
    static int pre_forth_signal = GlobalVariable::GetInstance().forth_signal[0];
    static int pre_for = GlobalVariable::GetInstance().for_signal[0];
    static int pre_back = GlobalVariable::GetInstance().back_signal[0];
    while(true){
        QThread::msleep(50);
        //读取寄存器的值
        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr1,nCount,GlobalVariable::GetInstance().forward_length,GlobalVariable::GetInstance().nNetId);
        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr4,nCount,GlobalVariable::GetInstance().backward_length,GlobalVariable::GetInstance().nNetId);

        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr2,nCount,GlobalVariable::GetInstance().first_signal,GlobalVariable::GetInstance().nNetId);
        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr11,nCount,GlobalVariable::GetInstance().second_signal,GlobalVariable::GetInstance().nNetId);
        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr12,nCount,GlobalVariable::GetInstance().third_signal,GlobalVariable::GetInstance().nNetId);
        nRes = H5u_Read_Soft_Elem_Int16(eType, Addr5,nCount,GlobalVariable::GetInstance().forth_signal,GlobalVariable::GetInstance().nNetId);

        nRes = H5u_Read_Soft_Elem_Int16(mType, Addr9,nCount,GlobalVariable::GetInstance().for_signal,GlobalVariable::GetInstance().nNetId);
        nRes = H5u_Read_Soft_Elem_Int16(mType, Addr10,nCount,GlobalVariable::GetInstance().back_signal,GlobalVariable::GetInstance().nNetId);

        // 处理每个信号
        processCameraSignal(pre_first_signal, GlobalVariable::GetInstance().first_signal, GlobalVariable::GetInstance().isNew1, GlobalVariable::GetInstance().Num1, GlobalVariable::GetInstance().photoQueues1, &ControlThread::StartFirstCamTask, &ControlThread::EndFirstCamTask, "第一个相机");
        processCameraSignal(pre_second_signal, GlobalVariable::GetInstance().second_signal, GlobalVariable::GetInstance().isNew2, GlobalVariable::GetInstance().Num2, GlobalVariable::GetInstance().photoQueues2, &ControlThread::StartSecondCamTask, &ControlThread::EndSecondCamTask, "第二个相机");
        processCameraSignal(pre_third_signal, GlobalVariable::GetInstance().third_signal, GlobalVariable::GetInstance().isNew3, GlobalVariable::GetInstance().Num3, GlobalVariable::GetInstance().photoQueues3, &ControlThread::StartThirdCamTask, &ControlThread::EndThirdCamTask, "第三个相机");
        processCameraSignal(pre_forth_signal, GlobalVariable::GetInstance().forth_signal, GlobalVariable::GetInstance().isNew4, GlobalVariable::GetInstance().Num4, GlobalVariable::GetInstance().photoQueues4, &ControlThread::StartForthCamTask, &ControlThread::EndForthCamTask, "第四个相机");

        //传送带换向时获取结果
        if(pre_for!=GlobalVariable::GetInstance().for_signal[0]&&pre_back!=GlobalVariable::GetInstance().back_signal[0]){
            emit GetResult();
            GlobalVariable::GetInstance().Num++;
            //重新计数
            GlobalVariable::GetInstance().Num1 = 0;
            GlobalVariable::GetInstance().Num2 = 0;
            GlobalVariable::GetInstance().Num3 = 0;
            GlobalVariable::GetInstance().Num4 = 0;
        }        

        pre_first_signal = GlobalVariable::GetInstance().first_signal[0];
        pre_second_signal = GlobalVariable::GetInstance().second_signal[0];
        pre_third_signal = GlobalVariable::GetInstance().third_signal[0];
        pre_forth_signal = GlobalVariable::GetInstance().forth_signal[0];

        pre_for = GlobalVariable::GetInstance().for_signal[0];
        pre_back = GlobalVariable::GetInstance().back_signal[0];
    }
}

void ControlThread::processCameraSignal(
    int& pre_signal,
    short* current_signal,
    bool& isNew,
    int& num,
    std::unordered_map<int, std::deque<cv::Mat>>& photoQueue,
    SignalEmitter  startTaskSignal,
    SignalEmitter  endTaskSignal,
    const QString& logMessageEnd
) {
    if (pre_signal == 1 && current_signal[0] == 0) {
        isNew = false;
        (this->*endTaskSignal)();
        // emit logMessage(logMessageEnd + "采集结束");
        //TODO:测长可能没有使用或存在问题
        GlobalVariable::GetInstance().for_signal[0] == 1 ?
            GlobalVariable::GetInstance().length1[GlobalVariable::GetInstance().Num1] = GlobalVariable::GetInstance().forward_length[0]:
            GlobalVariable::GetInstance().length2[GlobalVariable::GetInstance().Num4] = GlobalVariable::GetInstance().backward_length[0];
    }
    if (pre_signal == 0 && current_signal[0] == 1) {
        num++;
        isNew = true;
        photoQueue[num] = std::deque<cv::Mat>();
        (this->*startTaskSignal)();
    }
}


//启动验证模式
void ControlThread::StartVerify(){
    short start[1];
    start[0] = 1;
    nRes = H5u_Write_Soft_Elem_Int16(eType, Addr7, nCount,start, GlobalVariable::GetInstance().nNetId);
}

//关闭验证模式
void ControlThread::StopVerify(){
    short stop[1];
    stop[0] = 1;
    nRes = H5u_Write_Soft_Elem_Int16(eType, Addr8, nCount,stop, GlobalVariable::GetInstance().nNetId);
}
