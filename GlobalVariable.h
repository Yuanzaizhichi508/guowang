#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H

#include <deque>
#include <QImage>
#include <QDebug>
#include <csignal>
#include "opencv.hpp"
#include <chrono>
#include "yoloOnnxDetector.hpp"
#include "detectSection.hpp"
#include "detectStream.hpp"
#include "config.hpp"
#include "threadPool.hpp"
#include "PaddleOnnxDetector.hpp"
#include <QMutex>

//定义全局变量
class GlobalVariable
{
private:
    GlobalVariable();

public:
    /*
     * PLC控制相关参数
    */
    short forward_length[1];   //正转检测物料长度（需要除以10得到真实长度） 地址D700
    short first_signal[1];   //第一组相机拍照信号 地址D740 值为0或1
    short backward_length[1];  //反转检测物料长度 地址D800
    short second_signal[1];  //第二组相机拍照信号 地址D840

    short third_signal[1]; //第三个相机拍照信号 地址D850
    short forth_signal[1]; //第四个相机拍照信号，地址D840

    short feedback_forward_signal[1];  //正转触发相机拍照信号反馈信号,D740清零 地址D100 值为0或1
    short feedback_backward_signal[1];  //反转触发相机拍照信号反馈信号 地址D102

    short for_signal[1]; //1正转
    short back_signal[1]; //1反转

    int nNetId = 0;
    int nIpPort = 502; //ip连接相关参数

    int Num1 = 0;//角钢个数
    int Num2 = 0;
    int Num3 = 0;
    int Num4 = 0;
    int Num = 1;

    bool  isNew1 = false;
    bool  isNew2 = false;
    bool  isNew3 = false;
    bool  isNew4 = false;

    std::unordered_map<int, std::deque<cv::Mat>> photoQueues1;
    std::unordered_map<int, std::deque<cv::Mat>> photoQueues2;
    std::unordered_map<int, std::deque<cv::Mat>> photoQueues3;
    std::unordered_map<int, std::deque<cv::Mat>> photoQueues4;

    std::unordered_map<int,int> length1;
    std::unordered_map<int,int> length2;
    /*
     * 相机控制相关参数
    */

    static GlobalVariable& GetInstance(){
        static GlobalVariable globalVariable;
        return globalVariable;
    };

};

#endif // GLOBALVARIABLE_H
