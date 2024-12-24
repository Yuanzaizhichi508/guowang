
#pragma once

#ifdef PUBLIC_EXPORT
#define MONTION_CONTROL_FUNC_EXPORT __declspec( dllexport )
#else
#define MONTION_CONTROL_FUNC_EXPORT __declspec( dllimport )
#endif


#include "Include.h"
#include <string>
using namespace std;
void strcount(string & s);

//运动模式
enum  ElemMotionModeType
{
	Elem_Motion_Mode_None = 0,  //0-，
	Elem_Motion_Mode_Rel = 1,   //1-相对定位，
	Elem_Motion_Mode_Abs = 2,   //2-绝对定位，
	Elem_Motion_Mode_Home = 3,  //3-回零
	Elem_Motion_Mode_Jog = 4,   //4-点动
};

//运动状态
enum  ElemMotionState
{
	 Elem_Motion_State_Standby = 0, //0-空闲 
	 Elem_Motion_State_Finish = 1,  //1-运动完成 
	 Elem_Motion_State_Moving = 2,  //2 运动中 
	 Elem_Motion_State_Error = 3,   //3-错误
	 Elem_Motion_State_Stoping = 4,   //4-停止
	  Elem_Motion_State_SuddenStop = 5,   //5-紧急停止
};



struct stAxisMotionInof
{
	int nTargetPosition;                 //目标绝对位置
	int nTargetDistance;                 //目标相对距离
	int nCurrentPosition;                //轴当前前位置
	int nLastPosition;                   //运动前轴的位置
	int nHomeOffset;                     //原点偏移
	ElemMotionModeType nMode;            //运动模式 1-相对定位，2-绝对定位，3-回零
	ElemMotionState nState;              //状态0-空闲 1-运动完成 2 运动中 3-错误
};


/******************************************************************************
 1.功能描述 :创建网络连接	                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : IpAddr:以太网IP地址，
              nNetId:网络链接编号,用于标记是第几条网络链接,取值范围0~255,默认0 
			  IpPort:以太网端口号,默认502(modbusTcp协议默认端口号为502)
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL Init_ETH(DWORD IpAddr,int nNetId = 0,int IpPort = 502);

/******************************************************************************
 1.功能描述 :创建网络连接	                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : sIpAddr:以太网IP地址，
              nNetId:网络链接编号,用于标记是第几条网络链接,取值范围0~255,默认0 
			  IpPort:以太网端口号,默认502(modbusTcp协议默认端口号为502)
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL Init_ETH_String(char* pIpAddr,int nNetId = 0,int IpPort = 502);


/******************************************************************************
 1.功能描述 :关闭网络连接                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL Exit_ETH(int nNetId = 0 );


/******************************************************************************
 1.功能描述 :设置超时时间，            
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 在建立连接前调用，如果建立连接前没有调用，默认超时时间为500ms
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL Set_ETH_TimeOut(int nTime,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置延时时间           
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 用于需要延时回复的场合，没有需要可以不调用
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL Set_ETH_Delay(int nTime,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置轴工作模式           
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  WorkMode：工作模式 0-定位模式   1-JOG模式  2-设置当前位置为零点  3-	扭矩模式

 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_WorkMode(unsigned short Axis,short WorkMode,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置轴的目标位置        
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Pos :目标位置
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Pos(unsigned short Axis,int nPos,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴的目标速度      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Vel :目标速度
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Vel(unsigned short Axis,int Vel,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴的加减速大小      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Tacc::加减速时间
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Tacc(unsigned short Axis,DWORD Tacc,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 启动轴定位       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          start：0-关闭    1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_DRVA(unsigned short Axis,BOOL Start,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 点动正转       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          JogPlus：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_JOG_Plus(unsigned short Axis,BOOL JogPlus,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 点动正转       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          JogMinus：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_JOG_Minus(unsigned short Axis,BOOL JogMinus,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 启动转矩功能 (%QX511.3)
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          bStart：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_Torque_Mod(unsigned short Axis,BOOL bStart=1,int nNetId = 0);

/******************************************************************************
 1.功能描述 : 启动变速功能 (%QX511.4)
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          bStart：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_Velchange(unsigned short Axis,BOOL bStart=1,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 扭矩值设定 %MW5022 %MW5023
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          nTorqueValue：扭矩值
 4.注意事项 : 
 ******************************************************************************/	
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Torque_Value(unsigned short Axis,int nTorqueValue,int nNetId = 0);



/******************************************************************************
 1.功能描述 :轴使能       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          enable：0-关闭   1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_ENAB(unsigned short Axis,BOOL enable,int nNetId = 0);


/******************************************************************************
 1.功能描述 :轴减速停止       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          stop：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Stop(unsigned short Axis,BOOL stop,int nNetId = 0);



/******************************************************************************
 1.功能描述 :轴紧急停止       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          stop：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_EStop(unsigned short Axis,BOOL estop,int nNetId = 0);



/******************************************************************************
 1.功能描述 :轴回原       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  home：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Home(unsigned short Axis,BOOL home,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 轴复位      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  rest：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Rest(unsigned short Axis,BOOL rest,int nNetId = 0);



/******************************************************************************
 1.功能描述 :读取轴状态       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  state：bit0-初始状态
					 bit 1-定位中
					 bit 2-回原中
					 bit 3-位置到达
				     bit 4-回原完成
					 bit 5-保留
					 bit 6-保留
					 bit 7-保留
					 bit 8-轴故障
					 bit 9-轴掉线
					 bit10 –伺服准备好
					 bit11 –伺服使能
					 bit 12 –停止
					 bit 13 –紧急停止
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Axis_State(unsigned short Axis,short* state,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 读取轴当前位置      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  prepos：轴当前位置
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Axis_PrePos(unsigned short Axis,int* prepos,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 读取轴当前速度    
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  prevel：轴当前速度
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Axis_PreVel(unsigned short Axis,int* prevel,int nNetId = 0);

/******************************************************************************
 1.功能描述 : 读取轴的实际转矩    
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  pActTorque：输出参数，存储轴的实际转矩指针
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Axis_Act_Torque(unsigned short Axis,int* pActTorque,int nNetId = 0);

/******************************************************************************
 1.功能描述 : 读取轴对应伺服的故障代码    
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  pErrorCode：输出参数，存储伺服错误代码数据指针
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Servo_Error_Code(unsigned short Axis, unsigned short* pErrorCode,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置PLC输出Y的状态
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			Num：Y元件编号(八进制)
			Num=0表示Y0
			Num=1表示Y1；
			……
			outstate：Y输出点当前状态
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_IO_Out(short Num,BOOL bOutState,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 轴回原模式
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  homeMode：(请参考实际使用的伺服驱动器说明书)
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_HomeMode(unsigned short Axis,short HomeMode,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 轴回原偏移
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			HomeRec：相对机械原点轴回原偏移量
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_HomeRec(unsigned short Axis,int HomeRec,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴回原速度
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			Axis：轴号 
			HomeSpeed：轴回原高速速度值
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_HomeSpeed(unsigned short Axis,int HomeSpeed,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴回原搜索速度
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			Axis：轴号 
			HomeSearch：轴回原搜索速度值
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_HomeSearch(unsigned short Axis,int HomeSearch,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 读取伺服IO状态
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Iostate:DI1~DI9端子状态读取
			  bit 0-正向限位
			  bit 1-反向限位
			  bit 2-保留
			  bit 3-保留
			  bit 4-保留
			  bit 5-保留
			  bit 6-保留
			  bit 7-原点开关
			  bit 8-保留
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Servo_IO_State(unsigned short Axis,int* iostate,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 写软元件
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  eType：软元件类型
			  ELEM_QX — QX元件
			  ELEM_MW— MW元件
			  ELEM_X — X元件(QX200.0-QX299.7)
			  ELEM_Y— Y元件(QX300.0-QX399.7)
			  nStartAddr:软元件起始地址（QX元件由于带小数点，地址需要乘以10去掉小数点，如QX10.1，请输入101，其它元件不用处理）
			  nCount：软元件个数
			  pValue：数据缓存区
 4.注意事项 : 如果是写位元件，每个位元件的值存储在一个字节中
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Write_Soft_Elem(SoftElemType eType,int nStartAddr,int nCount,BYTE* pValue,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 读软元件
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  eType：软元件类型  ELEM__QX — QX元件 ELEM_MW— MW元件 ELEM_X — X元件 ELEM_Y— Y元件
			  nStartAddr:软元件起始地址
			  nCount：软元件个数
			  pValue：返回数据缓存区
 4.注意事项 : 如果是读位元件，每个位元件的值存储在一个字节中，pValue数据缓存区字节数必须是8的整数倍
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Read_Soft_Elem(SoftElemType eType,int nStartAddr,int nCount,BYTE* pValue,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 启动轴相对定位
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  start：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Start_Axis_DRVI(unsigned short Axis,BOOL start,int nNetId = 0);

/******************************************************************************
 1.功能描述 : 设置轴的缩放比例
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis:轴号号
			  fScale：比例值(0.001-1000)
 4.注意事项 : 如果从未设置比例，则默认值为1.0
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Scale(unsigned short nAxis, float fScale);

/******************************************************************************
 1.功能描述 : 设置轴的阈值
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis:轴号号
			 nThreshold ：阈值
 4.注意事项 : 如果从未设置阈值，则默认值为50
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int  Set_Axis_Threshold (unsigned short nAxis, int nThreshold);

/******************************************************************************
 1.功能描述 : 设置所有轴的缩放比例
 2.返 回 值 :1 成功  0 失败
 3.参    数 : fScale：比例值(0.001-1000)
 4.注意事项 : 如果从未设置比例，则默认值为1.0 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_All_Axis_Scale( float fScale);

/******************************************************************************
 1.功能描述 : 设置所有轴的阈值
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nThreshold ：阈值
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int  Set_All_Axis_Threshold (int nThreshold);


/******************************************************************************
 1.功能描述 : 绝对定位指令
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis：轴号
			  nPos:  目标位置
			  nVel： 速度
 			  nAcc： 加速度
4.注意事项 : 调用前请先调用一次Set_Axis_Scale()和Set_Axis_Threshold()分别进行
			  比例和阈值设置，这两个参数可以掉电保存
			  当轴的运动状态-stAxisMotionInof->nState为Elem_Motion_State_Moving时，函数调用失败，返回0
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Move_Abs(unsigned short nAxis, int nPos, int nVel, int nAcc, int nNetId = 0);

/******************************************************************************
 1.功能描述 : 相对定位指令
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis：轴号
			  nDistance：轴号
			  nVel： 速度
 			  nAcc： 加速度
4.注意事项 : 调用前请先调用一次Set_Axis_Scale()和Set_Axis_Threshold()分别进行
             比例和阈值设置，这两个参数可以掉电保存
             当轴的运动状态-stAxisMotionInof->nState为Elem_Motion_State_Moving时，函数调用失败，返回0
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Move_Rel(unsigned short nAxis, int nDistance, int nVel, int nAcc, int nNetId = 0);

/******************************************************************************
 1.功能描述 : 回零指令
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis：轴号
			  nOffset：原点偏移
			  nVel： 速度
 			  nAcc： 加速度
 4.注意事项 : 调用前请先调用一次Set_Axis_Scale()和Set_Axis_Threshold()分别进行
              比例和阈值设置，这两个参数可以掉电保存.
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Axis_Move_Home(unsigned short nAxis, int nOffset, int nSpeed, int nSearch, int nNetId = 0);


/******************************************************************************
 1.功能描述 : 获取轴的运动状态
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nAxis：轴号
			  pMotionInof：存储轴状态信息的结构体指针

 4.注意事项 : 调用Axis_Move_Abs、Axis_Move_Rel、Axis_Move_Home其中任何一个，
              调用本函数查询运动状态
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_Aixs_Motion_State(unsigned short nAxis, stAxisMotionInof *pMotionInof, int nNetId = 0);



/******************************************************************************
 1.功能描述 : 设置伺服输出
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  Axis:轴号
			  enable:使能
			  outstate:输出状态
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Set_Servo_Out_State(unsigned short Axis,BOOL enable,int outstate, int nNetId = 0);


/******************************************************************************
 1.功能描述 :获取PLC输出IO元件Y的状态      
 2.返 回 值 :1 调用成功 0 调用失败
 3.参    数 :Num--Y元件地址，
             bIoState--制定元件的状态
            nNetId--网络链接编号 
 4.注意事项 : Num为8进制数据
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_IO_Out(short Num, bool *bIoState, int nNetId = 0);


/******************************************************************************
 1.功能描述 :获取PLC输入IO X的状态      
 2.返 回 值 :1 调用成功 0 调用失败
 3.参    数 :Num--X元件地址，
			 bIoState--制定元件的状态
            nNetId--网络链接编号 
 4.注意事项 : Num为8进制数据
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int Get_IO_In(short Num, bool *bIoState, int nNetId = 0);




 /******************************************************************************
 1.功能描述    : 设置轴数
 2.参    数    : wLinkId--连接号（每连接对应一台PLC）  wNum--运动控制轴的数量
 3.返 回 值    : 1 成功 0 失败
 4.注意事项    : 
******************************************************************************/
 MONTION_CONTROL_FUNC_EXPORT int Set_Axis_Num(WORD wNum, int wLinkId=0);



 //以下是H3u的API

/******************************************************************************
 1.功能描述 :创建网络连接	                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : IpAddr:以太网IP地址，
              nNetId:网络链接编号,用于标记是第几条网络链接,取值范围0~255,默认0 
			  IpPort:以太网端口号,默认502(modbusTcp协议默认端口号为502)
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL H3u_Init_ETH(DWORD IpAddr,int nNetId = 0,int IpPort = 502);


/******************************************************************************
 1.功能描述 :创建网络连接	                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : sIpAddr:以太网IP地址，
              nNetId:网络链接编号,用于标记是第几条网络链接,取值范围0~255,默认0 
			  IpPort:以太网端口号,默认502(modbusTcp协议默认端口号为502)
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL H3u_Init_ETH_String(char* pIpAddr,int nNetId = 0,int IpPort = 502);


/******************************************************************************
 1.功能描述 :关闭网络连接                  
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL H3u_Exit_ETH(int nNetId = 0 );


/******************************************************************************
 1.功能描述 :设置超时时间，            
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 在建立连接前调用，如果建立连接前没有调用，默认超时时间为500ms
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL H3u_Set_ETH_TimeOut(int nTime,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置延时时间           
 2.返 回 值 :TRUE 成功  FALSE 失败
 3.参    数 : nNetId:网络链接编号,与Init_ETH（）调用的ID一样
 4.注意事项 : 用于需要延时回复的场合，没有需要可以不调用
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT BOOL H3u_Set_ETH_Delay(int nTime,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置轴工作模式           
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          WorkMode：工作模式 0-定位模式   1-JOG模式  2-设置当前位置为零点 
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_WorkMode(unsigned short Axis,short WorkMode,int nNetId = 0);


/******************************************************************************
 1.功能描述 :设置轴的目标位置        
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Pos :目标位置
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_Pos(unsigned short Axis,int nPos,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴的目标速度      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Vel :目标速度
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_Vel(unsigned short Axis,int Vel,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 设置轴的加减速时间      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          Tacc::加减速时间
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_Tacc(unsigned short Axis,DWORD Tacc,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 启动轴定位       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          start：0-关闭    1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Start_Axis_DRVA(unsigned short Axis,BOOL Start,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 点动正转       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          JogPlus：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Start_Axis_JOG_Plus(unsigned short Axis,BOOL JogPlus,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 点动正转       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          JogMinus：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Start_Axis_JOG_Minus(unsigned short Axis,BOOL JogMinus,int nNetId = 0);


/******************************************************************************
 1.功能描述 :轴使能       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          enable：0-关闭   1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Axis_ENAB(unsigned short Axis,BOOL enable,int nNetId = 0);


/******************************************************************************
 1.功能描述 :轴减速停止       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          stop：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Axis_Stop(unsigned short Axis,BOOL stop,int nNetId = 0);



/******************************************************************************
 1.功能描述 :轴紧急停止       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          stop：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Axis_EStop(unsigned short Axis,BOOL estop,int nNetId = 0);


/******************************************************************************
 1.功能描述 :MC_Halt停止       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
	          stop：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT  int Axis_MC_Halt_Stop(unsigned short Axis,BOOL estop,int nNetId = 0);



/******************************************************************************
 1.功能描述 :轴回原       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  home：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Axis_Home(unsigned short Axis,BOOL home,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 轴复位      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  rest：0-关闭  1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Axis_Rest(unsigned short Axis,BOOL rest,int nNetId = 0);



/******************************************************************************
 1.功能描述 :读取轴状态       
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  state：bit0-初始状态
					 bit 1-定位中
					 bit 2-回原中
					 bit 3-位置到达
				     bit 4-回原完成
					 bit 5-定位参数错误
					 bit 6-回原参数错误
					 bit 7-jog中参数错误
					 bit 8-轴故障
					 bit 9-轴掉线
					 bit10 –伺服准备好
					 bit11 –伺服使能
					 bit12 –停止
					 bit13 –紧急停止
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_Axis_State(unsigned short Axis,short* state,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 读取轴当前位置      
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  prepos：轴当前位置
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_Axis_PrePos(unsigned short Axis,int* prepos,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 读取轴当前速度    
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  prevel：轴当前速度
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_Axis_PreVel(unsigned short Axis,int* prevel,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 设置PLC输出Y的状态
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			Num：Y元件编号(八进制)
			Num=0表示Y0
			Num=1表示Y1
			……
			outstate：Y输出点当前状态
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_IO_Out(short Num,BOOL bOutState,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 设置轴回原模式
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Axis：轴号 
			  homeMode：
			  0-正向回零,减速点、原点为原点开关
			  1-反向回零,减速点、原点为原点开关
			  2-正向回零,减速点、原点为电机Z信号
			  3-反向回零,减速点、原点为电机Z信号
			  4-正向回零,减速点为原点开关、原点为电机Z信号
			  5-反向回零,减速点为原点开关、原点为电机Z信号
			  6-正向回零,减速点、原点为正向超程开关
			  7-反向回零,减速点、原点为正向超程开关
			  8-正向回零,减速点为正向超程开关、原点为电机Z信号
			  9-反向回零,减速点为正向超程开关、原点为电机Z信号
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_HomeMode(unsigned short Axis,short HomeMode,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 轴回原偏移
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			HomeRec：相对机械原点轴回原偏移量
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Axis_HomeRec(unsigned short Axis,int HomeRec,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 读取伺服IO状态
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
              Iostate:DI1~DI9端子状态读取
			  bit 0-正向限位
			  bit 1-反向限位
			  bit 2-保留
			  bit 3-保留
			  bit 4-保留
			  bit 5-保留
			  bit 6-保留
			  bit 7-原点开关
			  bit 8-保留
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_Servo_IO_State(unsigned short Axis,int* iostate,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 写软元件
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  eType：软元件类型
				  REGI_H3U_Y    = 0x20,       //Y元件的定义	
				  REGI_H3U_X    = 0x21,		//X元件的定义							
				  REGI_H3U_S    = 0x22,		//S元件的定义				
				  REGI_H3U_M    = 0x23,		//M元件的定义							
				  REGI_H3U_TB   = 0x24,		//T位元件的定义				
				  REGI_H3U_TW   = 0x25,		//T字元件的定义				
				  REGI_H3U_CB   = 0x26,		//C位元件的定义				
				  REGI_H3U_CW   = 0x27,		//C字元件的定义				
				  REGI_H3U_DW   = 0x28,		//D字元件的定义				
				  REGI_H3U_CW2  = 0x29,	    //C双字元件的定义
				  REGI_H3U_SM   = 0x2a,		//SM
				  REGI_H3U_SD   = 0x2b,		//SD
				  REGI_H3U_R    = 0x2c		//SD
			  nStartAddr:软元件起始地址
			  nCount：软元件个数
			  pValue：数据缓存区
 4.注意事项 : 如果是写位元件，每个位元件的值存储在一个字节中，
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Write_Soft_Elem(SoftElemType eType,int nStartAddr,int nCount,BYTE* pValue,int nNetId = 0);


/******************************************************************************
 1.功能描述 : 读软元件
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  eType：软元件类型  
				  REGI_H3U_Y    = 0x20,     //Y元件的定义	
				  REGI_H3U_X    = 0x21,		//X元件的定义							
				  REGI_H3U_S    = 0x22,		//S元件的定义				
				  REGI_H3U_M    = 0x23,		//M元件的定义							
				  REGI_H3U_TB   = 0x24,		//T位元件的定义				
				  REGI_H3U_TW   = 0x25,		//T字元件的定义				
				  REGI_H3U_CB   = 0x26,		//C位元件的定义				
				  REGI_H3U_CW   = 0x27,		//C字元件的定义				
				  REGI_H3U_DW   = 0x28,		//D字元件的定义				
				  REGI_H3U_CW2  = 0x29,	    //C双字元件的定义
				  REGI_H3U_SM   = 0x2a,		//SM
				  REGI_H3U_SD   = 0x2b,		//SD
				  REGI_H3U_R    = 0x2c		//SD
			  nStartAddr:软元件起始地址
			  nCount：软元件个数
			  pValue：返回数据缓存区
 4.注意事项 : 如果是读位元件，每个位元件的值存储在一个字节中，pValue数据缓存区字节数必须是8的整数倍
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Read_Soft_Elem(SoftElemType eType,int nStartAddr,int nCount,BYTE* pValue,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 启动轴相对定位
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  start：0-关闭 1-启动
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Start_Axis_DRVI(unsigned short Axis,BOOL start,int nNetId = 0);



/******************************************************************************
 1.功能描述 : 设置伺服输出
 2.返 回 值 :1 成功  0 失败
 3.参    数 : nNetId:网络链接编号
			  Axis:轴号
			  enable:使能
			  outstate:输出状态
 4.注意事项 : 
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Set_Servo_Out_State(int Axis,BOOL enable,short outstate, int nNetId = 0);


/******************************************************************************
 1.功能描述 :获取PLC输出IO元件Y的状态      
 2.返 回 值 :指定元件的状态
 3.参    数 :Num--Y元件地址，
            nNetId--网络链接编号 
 4.注意事项 : Num为8进制数据
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_IO_Out(short Num,bool *bIoState, int nNetId = 0);


/******************************************************************************
 1.功能描述 :获取PLC输入IO X的状态      
 2.返 回 值 :指定元件的状态
 3.参    数 :Num--X元件地址，
            nNetId--网络链接编号 
 4.注意事项 : Num为8进制数据
******************************************************************************/
MONTION_CONTROL_FUNC_EXPORT int H3u_Get_IO_In(short Num,bool *bIoState, int nNetId = 0);


