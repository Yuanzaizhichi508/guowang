#ifndef COMMON_H
#define COMMON_H

#include <QMessageBox>
#include <QComboBox>
#include <QString>
#include <QTimer>
#include "GxIAPI.h"
#include "DxImageProc.h"
#include "GxEx.h"

#define FRAMERATE_INCREMENT     0.1
#define EXPOSURE_INCREMENT      1
#define GAIN_INCREMENT          0.1
#define WHITEBALANCE_DECIMALS   4
#define WHITEBALANCE_INCREMENT  0.0001

// 官方例程给的一些工具函数
#define RELEASE_ALLOC_MEM(obj)    \
if (obj != NULL)    \
    {   \
            delete obj;     \
            obj = NULL;     \
    }

#define RELEASE_ALLOC_ARR(obj) \
if (obj != NULL)    \
    {   \
            delete[] obj;   \
            obj = NULL;     \
    }

#define GX_VERIFY(emStatus) \
if(emStatus != GX_STATUS_SUCCESS) \
    { \
            ShowErrorString(emStatus); \
            return; \
    }

void ShowErrorString(GX_STATUS);
GX_STATUS InitComboBox(GX_DEV_HANDLE, QComboBox*, GX_FEATURE_ID);

#endif // COMMON_H
