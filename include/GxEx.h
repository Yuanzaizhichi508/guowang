#ifndef GX_EX_H
#define GX_EX_H
#include "GxIAPI.h"

///相机参数结构体
typedef struct CAMER_INFO
{
	BITMAPINFO					*pBmpInfo;		  ///< BITMAPINFO 结构指针，显示图像时使用
	BYTE						*pImageBuffer;	  ///< 指向经过处理后的图像数据缓冲区
	BYTE                        *pRawBuffer;      ///< 指向原始RAW图缓冲区
	char						chBmpBuf[2048];	  ///< BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区	
	int64_t                     nPayLoadSise;     ///< 图像块大小
	int64_t					    nImageWidth;	  ///< 图像宽度
	int64_t					    nImageHeight;	  ///< 图像高度	
	int64_t					    nBayerLayout;	  ///< Bayer排布格式
	bool						bIsColorFilter;	  ///< 判断是否为彩色相机
	BOOL						bIsOpen;		  ///< 相机已打开标志
	BOOL						bIsSnap;		  ///< 相机正在采集标志
	float						fFps;			  ///< 帧率
}CAMER_INFO;
	
#endif