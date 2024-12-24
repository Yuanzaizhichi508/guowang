#ifndef CONTROLTHREAD_H
#define CONTROLTHREAD_H

#include <QThread>
#include <QTimer>
#include <afxwin.h>
#include "Hc_Modbus_Api.h"
#include "GlobalVariable.h"

class ControlThread : public QThread
{
    Q_OBJECT
public:
    ControlThread();

    using SignalEmitter = void (ControlThread::*)();

    void StartVerify();

    void StopVerify();

    void processCameraSignal(
        int& pre_signal,
        short* current_signal,
        bool& isNew,
        int& num,
        std::unordered_map<int, std::deque<cv::Mat>>& photoQueue,
        SignalEmitter  startTaskSignal,
        SignalEmitter  endTaskSignal,
        const QString& logMessageEnd);


signals:

    void StartFirstCamTask();
    void EndFirstCamTask();

    void StartSecondCamTask();
    void EndSecondCamTask();

    void StartThirdCamTask();
    void EndThirdCamTask();

    void StartForthCamTask();
    void EndForthCamTask();

    void logMessage(const QString &message);

    void GetResult();



private:
    SoftElemType eType = REGI_H5U_D;  //寄存器类型
    SoftElemType mType = REGI_H5U_M;

    int Addr1 = 700;
    int Addr2 = 740;
    int Addr3 = 100;

    int Addr4 = 800;
    int Addr5 = 840;
    int Addr6 = 102;  //寄存器地址

    int Addr7 = 750;
    int Addr8 = 760;

    int Addr9 = 102;
    int Addr10 = 103;

    int Addr11 = 850;
    int Addr12 = 960;

    int nCount = 1;  //寄存器个数，一般为1
    int nRes; //读写结果返回


protected:
    void run();
};

#endif // CONTROLTHREAD_H
