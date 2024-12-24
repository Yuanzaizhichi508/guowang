#include "Common.h"

void ShowErrorString(GX_STATUS error_status)
{
    char*     error_info = NULL;
    size_t    size        = 0;
    GX_STATUS emStatus     = GX_STATUS_ERROR;

    emStatus = GXGetLastError(&error_status, NULL, &size);

    try
    {
        error_info = new char[size];
    }
    catch (std::bad_alloc& e)
    {
        QMessageBox::about(NULL, "Error", "Alloc error info Faild!");
        return;
    }

    // 获得错误信息并显示
    emStatus = GXGetLastError (&error_status, error_info, &size);

    if (emStatus != GX_STATUS_SUCCESS)
    {
        QMessageBox::about(NULL, "Error", "  Interface of GXGetLastError call failed! ");
    }
    else
    {
        QMessageBox::about(NULL, "Error", QString("%1").arg(QString(QLatin1String(error_info))));
    }

    if (NULL != error_info)
    {
        delete[] error_info;
        error_info = NULL;
    }

    return;
}

GX_STATUS InitComboBox(GX_DEV_HANDLE m_hDevice, QComboBox* qobjComboBox, GX_FEATURE_ID emFeatureID)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    uint32_t ui32EntryNums = 0;
    GX_ENUM_DESCRIPTION* pEnumDescription;
    size_t nSize;
    int64_t i64CurrentValue = 0;

    emStatus = GXGetEnumEntryNums(m_hDevice, emFeatureID, &ui32EntryNums);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        return emStatus;
    }

    try
    {
        pEnumDescription = new GX_ENUM_DESCRIPTION[ui32EntryNums];
        nSize =  ui32EntryNums * sizeof(GX_ENUM_DESCRIPTION);
    }
    catch (std::bad_alloc &e)
    {
        QMessageBox::about(NULL, "Error", "Allocate Enum Description Failed!");
        return emStatus;
    }

    emStatus = GXGetEnumDescription(m_hDevice, emFeatureID, pEnumDescription, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        RELEASE_ALLOC_ARR(pEnumDescription);
        return emStatus;
    }

    emStatus = GXGetEnum(m_hDevice, emFeatureID, &i64CurrentValue);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        RELEASE_ALLOC_ARR(pEnumDescription);
        return emStatus;
    }

    for (uint32_t i = 0; i < ui32EntryNums; i++)
    {
        qobjComboBox->insertItem(ui32EntryNums, pEnumDescription[i].szSymbolic, QVariant::fromValue(pEnumDescription[i].nValue));
    }

    qobjComboBox->setCurrentIndex(qobjComboBox->findData(QVariant::fromValue(i64CurrentValue)));

    RELEASE_ALLOC_ARR(pEnumDescription);

    return GX_STATUS_SUCCESS;
}
