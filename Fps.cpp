//--------------------------------------------------------------------------------
/**
\file     Fps.h
\brief    CFps类的实现文件
*/
//----------------------------------------------------------------------------------
#include "StdAfx.h"
#include "Fps.h"


//----------------------------------------------------------------------------------
/**
\brief  构造函数
*/
//----------------------------------------------------------------------------------

CFps::CFps(void)
    :m_nFrameCount(0)
    ,m_dBeginTime(0)
    ,m_dEndTime(0)
    ,m_nTotalFrameCount(0)
    ,m_dFps(0)
{
    //启动计时操作
    m_objStopWatch.Start();
}

//----------------------------------------------------------------------------------
/**
\brief  析构函数
*/
//----------------------------------------------------------------------------------

CFps::~CFps(void)
{
}

//----------------------------------------------------------------------------------
/**
\brief  获取最近一次的帧率
\param  pFrame  当前帧图像
*/
//----------------------------------------------------------------------------------
double CFps::GetFps(void)
{
    //返回当前的帧率
    return m_dFps;
}


//----------------------------------------------------------------------------------
/**
\brief  增加帧数
*/
//----------------------------------------------------------------------------------

void CFps::IncreaseFrameNum(void)
{
    //累积帧数
    m_nTotalFrameCount++;

    //增加帧数
    m_nFrameCount++;

    //更新时间间隔
    m_dEndTime = m_objStopWatch.Stop();
}


//----------------------------------------------------------------------------------
/**
\brief  更新帧率
        如果该函数被调用的频率超过了帧频率，则帧率会降为零
*/
//----------------------------------------------------------------------------------
void CFps::UpdateFps(void)
{
    //计算时间间隔
    double dInterval = m_dEndTime - m_dBeginTime;

    //时间间隔大于零
    if(dInterval > 0)
    {
        m_dFps = 1000.0 * m_nFrameCount / dInterval;
        m_nFrameCount   = 0;              //累积帧数清零
        m_dBeginTime   = m_dEndTime;      //更新起始时间
    }
    else if(dInterval == 0) //时间间隔等于零
    {
        m_dFps = 0;
    }
    else{}

}
