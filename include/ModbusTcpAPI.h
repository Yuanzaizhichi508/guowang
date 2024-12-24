#pragma  once

#ifdef PUBLIC_EXPORT
#define MODBUS_TCP_DLL_FUNC_EXPORT __declspec( dllexport )
#else
#define MODBUS_TCP_DLL_FUNC_EXPORT __declspec( dllimport )
#endif


#define MAX_AXIS_AM600   30
#define MAX_AXIS_H3U     12
struct AddrTypeInfo
{
	DWORD dwAddrStart;		//起始地址（Modbus地址）
	DWORD dwAddrLen;		//地址总的个数
	DWORD dwAddrType;		//地址类型代表的字节数, 0位类型，2：WORD型，4：双字型
};


 struct ModubsMultWriteData
{
	ModubsMultWriteData()
	{
		wAxis = -1;//轴号
		wStar = -1;//1启动，0无效
		dwPos = -1;////设置轴的目标位置
		dwVel = -1;//设置轴的目标速度
		dwTacc = -1;//置轴的加减速时间
	}
	WORD wAxis;//轴号
	WORD wStar;//1启动，0无效
	DWORD dwPos;////设置轴的目标位置
	DWORD dwVel;//设置轴的目标速度
	DWORD dwTacc;//置轴的加减速时间
};

 static AddrTypeInfo g_dwReadAddr_H3u_Y[]={
	 {6000, 15, 2},		//输出
	 {6020, 15, 2},		//输入
	//{6000, 5, 2},		//输出
	//{6020, 5, 2},		//输出
	 {0, 0,	 0},
 };

 static AddrTypeInfo g_dwReadAddr_H3u[]={
	 {5000, 17, 2},		//输出
	 {5100, 17, 2},		//
	 {5200, 17, 2},		//输出
	 {5300, 17, 2},		//
	 {5400, 17, 2},		//输出
	 {5500, 17, 2},		//
	 {5600, 17, 2},		//输出
	 {5700, 17, 2},		//
	 //{5800, 17, 2},		//输出
	 //{5900, 17, 2},		//
	 {6100, 17, 2},		//输出
	 {6200, 17, 2},		//
	 {6300, 17, 2},		//输出
	 {6400, 17, 2},		//
	 //{6500, 17, 2},		//输出
	 {0, 0, 0},		//
	 {0, 0, 0},		//输出
	 {0, 0, 0},		//
	 {0, 0, 0},		//输出
	 {0, 0, 0},		//
	 {0, 0, 0},
 };

 static AddrTypeInfo g_dwReadAddr_Y[]={
	 {8000, 63, 2},		//输入
	 {9000, 63, 2},		//输出
	 {0, 0,	 0},
 };

 //static AddrTypeInfo g_dwReadAddr[]={
	// {5000, 17, 2},		//输出
	// {5100, 17, 2},		//
	// {5200, 17, 2},		//输出
	// {5300, 17, 2},		//
	// {5400, 17, 2},		//输出
	// {5500, 17, 2},		//
	// {5600, 17, 2},		//输出
	// {5700, 17, 2},		//
	// {5800, 17, 2},		//输出
	// {5900, 17, 2},		//
	// {6000, 17, 2},		//输出
	// {6100, 17, 2},		//
	// {6200, 17, 2},		//输出
	// {6300, 17, 2},		//
	// {6400, 17, 2},		//输出
	// {6500, 17, 2},		//
	// {6600, 17, 2},		//输出
	// {6700, 17, 2},		//
	// {6800, 17, 2},		//输出
	// {6900, 17, 2},		//	 
	// {7000, 17, 2},		//输出
	// {7100, 17, 2},		//
	// {7200, 17, 2},		//输出
	// {7300, 17, 2},		//
	// {7400, 17, 2},		//输出
	// {7500, 17, 2},		//
	// {7600, 17, 2},		//输出
	// {7700, 17, 2},		//
	// {7800, 17, 2},		//输出
	// {7900, 17, 2},		//
	// {0, 0,	 0},
 //};
 static AddrTypeInfo g_dwReadAddr[]={
	 {5000, 24, 2},		//01输出
	 {5100, 24, 2},		//02
	 {5200, 24, 2},		//03输出
	 {5300, 24, 2},		//04
	 {5400, 24, 2},		//05输出
	 {5500, 24, 2},		//06
	 {5600, 24, 2},		//07输出
	 {5700, 24, 2},		//08
	 {5800, 24, 2},		//09输出
	 {5900, 24, 2},		//10
	 {6000, 24, 2},		//11输出
	 {6100, 24, 2},		//12
	 {6200, 24, 2},		//13输出
	 {6300, 24, 2},		//14
	 {6400, 24, 2},		//15输出
	 {6500, 24, 2},		//16
	 {6600, 24, 2},		//17输出
	 {6700, 24, 2},		//18
	 {6800, 24, 2},		//19输出
	 {6900, 24, 2},		//20	 
	 {7000, 24, 2},		//21输出
	 {7100, 24, 2},		//22
	 {7200, 24, 2},		//23输出
	 {7300, 24, 2},		//24
	 {7400, 24, 2},		//25输出
	 {7500, 24, 2},		//26
	 {7600, 24, 2},		//27输出
	 {7700, 24, 2},		//28
	 {7800, 24, 2},		//29输出
	 {7900, 24, 2},		//30
	 {0, 0,	 0},
 };


 static AddrTypeInfo g_dwWiteBitAddr[]={
	 {500*8, 10, 0},		
	 {510*8, 10, 0},		
	 {520*8, 10, 0},		
	 {530*8, 10, 0},		
	 {540*8, 10, 0},		
	 {550*8, 10, 0},		
	 {560*8, 10, 0},		
	 {570*8, 10, 0},		
	 {580*8, 10, 0},		
	 {590*8, 10, 0},		
	 {600*8, 10, 0},		
	 {610*8, 10, 0},		
	 {620*8, 10, 0},		
	 {630*8, 10, 0},		
	 {640*8, 10, 0},		
	 {650*8, 10, 0},		
	 {660*8, 10, 0},		
	 {670*8, 10, 0},		
	 {680*8, 10, 0},		
	 {690*8, 10, 0},	 
	 {700*8, 10, 0},		
	 {710*8, 10, 0},		
	 {720*8, 10, 0},		
	 {730*8, 10, 0},		
	 {740*8, 10, 0},		
	 {750*8, 10, 0},		
	 {760*8, 10, 0},		
	 {770*8, 10, 0},		
	 {780*8, 10, 0},		
	 {790*8, 10, 0},			
	 {0, 0,	 0},
 };

 enum  emPlcDeviceType
 {
	 PLC_DEVICE_AM600 = 0,     //QX元件
	 PLC_DEVICE_H3U = 1        //QX元件
 };

 
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpConnect(int nport, int IpPort, DWORD IpAddress);
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL  mbtcpConnectString(int nport, int IpPort, char* pIpAddr);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpClose(int nport);

 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpSetTimeout(int nport,int mtime);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpSetDelay(int nport,int mtime);

 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn01(int nport, int node, int address, int Count,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn02(int nport, int node, int address, int Count,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn03(int nport, int node, int address, int Count,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn04(int nport, int node, int address, int Count,int* RxdBuffer,int* RxdLength);

 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn05(int nport, int node, int address, int Count,int value,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn06(int nport, int node, int address, int Count,int value,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn15(int nport, int node, int address, int Count,WORD* TxdBuffer,int* RxdBuffer,int* RxdLength);
 MODBUS_TCP_DLL_FUNC_EXPORT int   mbtcpfcn16(int nport, int node, int address, int Count,WORD* TxdBuffer,int* RxdBuffer,int* RxdLength);
 

 MODBUS_TCP_DLL_FUNC_EXPORT AddrTypeInfo* GetPlcDataFromLocalBuf(int wLinkId);

 MODBUS_TCP_DLL_FUNC_EXPORT AddrTypeInfo* GetPlcDataFromLocalBuf_Y(int wLinkId);

 //MODBUS_TCP_DLL_FUNC_EXPORT void AddStarAxisCmd(int wLinkId, ModubsMultWriteData* pModubsMultWriteData);




 int DataExchange(int nport,const char* pBuffer,int* RxdBuffer,int* RxdLength);




 /******************************************************************************
 1.功能描述    : 设置轴数
 2.参    数    : wLinkId--连接号（每连接对应一台PLC）  wNum--运动控制轴的数量
 3.返 回 值    : 无
 4.注意事项    : 
******************************************************************************/
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL _SetAxisNum( WORD wNum,int wLinkId);

 
 /******************************************************************************
 1.功能描述    : 设置位元件状态
 2.参    数    : wLinkId--连接号（每连接对应一台PLC）  wAddr--寄存器地址（modbus编址）
                 bState-位元件状态
 3.返 回 值    : TRUE--成功 FALSE--失败  
 4.注意事项    : 
******************************************************************************/
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL SetCoilState(int wLinkId,WORD wAddr, BOOL bState);

 
 /******************************************************************************
 1.功能描述    : 设置字（16位）元件状态
 2.参    数    : wLinkId--连接号（每连接对应一台PLC）  wAddr--寄存器地址（modbus编址）
                 wWordValue--16位字元件值
 3.返 回 值    : TRUE--成功 FALSE--失败  
 4.注意事项    : 
******************************************************************************/
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL SetWord(int wLinkId,WORD wAddr, WORD wWordValue);

  
 /******************************************************************************
 1.功能描述    : 设置双字（32位）元件状态
 2.参    数    : wLinkId--连接号（每连接对应一台PLC）  wAddr--寄存器地址（modbus编址）
                 wWordValue--16位字元件值
 3.返 回 值    : TRUE--成功 FALSE--失败  
 4.注意事项    : 低16位在前，高16位在后
******************************************************************************/
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL SetDoubleWord(int wLinkId,WORD wAddr, DWORD dwWordValue);

 /******************************************************************************
 1.功能描述    : 写运动指令
 2.参    数    : pValue：存储参数值的指针
				pAddr:存储参数值寄存器地址的指针
				wPramConst：参数个数
                wStarteBitAddr：运动启动位元件地址
 3.返 回 值    : TRUE--成功 FALSE--失败  
 4.注意事项    : 低16位在前，高16位在后
******************************************************************************/
 MODBUS_TCP_DLL_FUNC_EXPORT BOOL WriteMotionCmd(int* pPramValue,WORD* pAddr, WORD wPramConst, WORD wStarteBitAddr,int nNetId);

 MODBUS_TCP_DLL_FUNC_EXPORT int   StartApiExProcess(int nport, int IpPort, DWORD IpAddress, emPlcDeviceType emType);

/*******************************************************************************
1.功能描述    : 批量写寄存器配置指令
2.参    数    : nByteCnt：位元件个数
nWordCnt：字元件个数
pAddrBuffer:元件地址
pRxdBuffer：返回数据
3.返 回 值    : 数据的长度 
4.注意事项    : 低16位在前，高16位在后
*********************************************************************************/

 MODBUS_TCP_DLL_FUNC_EXPORT int  BatchSetReadRegCfg(int nNetId, int nByteCnt, int nWordCnt,WORD* pAddrBuffer,int* pRxdBuffer);

 MODBUS_TCP_DLL_FUNC_EXPORT int  BatchReadReg(int nNetId,int nRcvLen,int* pRxdBuffer);

 extern BOOL  g_bIsStarted;



