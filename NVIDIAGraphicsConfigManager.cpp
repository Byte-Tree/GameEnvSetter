#include "NVIDIAGraphicsConfigManager.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QJsonDocument>
#include <qjsonarray.h>
#include <qjsonobject.h>

#include "NvApiDriverSettings.h"

// NVAPI 字符串长度定义
#define NVAPI_SHORT_STRING_LENGTH 64
#define NVAPI_BINARY_DATA_MAX_SIZE 4096

// 静态成员初始化
bool NVIDIAGraphicsConfigManager::isNvAPIInitialized = false;
NvDRSSessionHandle NVIDIAGraphicsConfigManager::globalSession = 0;

void NVIDIAGraphicsConfigManager::PrintError(NvAPI_Status status)
{
    NvAPI_ShortString szDesc = {0};
    NvAPI_GetErrorMessage(status, szDesc);
    printf(" NVAPI error: %s\n", szDesc);
}

QList<QPair<QString, QPair<NvU32, NvU32>>> NVIDIAGraphicsConfigManager::getAllNvidiaSettings() {
    QList<QPair<QString, QPair<NvU32, NvU32>>> settingsList;

    if (!initializeNvAPI()) {
        qWarning() << "NVIDIA API 初始化失败";
        return settingsList;
    }

    NvU32 settingCount = 0;
    NvAPI_Status status = NvAPI_DRS_EnumAvailableSettingIds(nullptr,&settingCount);
    if (status != NVAPI_OK || settingCount == 0) {
        qWarning() << "无法获取设置项数量，错误码:" << status;
        return settingsList;
    }

    NvU32* settingIds = new NvU32[settingCount];
    status = NvAPI_DRS_EnumAvailableSettingIds(settingIds,&settingCount);
    if (status != NVAPI_OK) {
        qWarning() << "无法枚举设置项ID，错误码:" << status;
        delete[] settingIds;
        return settingsList;
    }

    for (NvU32 i = 0; i < settingCount; ++i) {
        NvAPI_UnicodeString settingName = {0};
        status = NvAPI_DRS_GetSettingNameFromId(settingIds[i], &settingName);
        if (status == NVAPI_OK) {
            QString name = QString::fromWCharArray(reinterpret_cast<wchar_t*>(settingName));
            NVDRS_SETTING currentSetting = queryNvidiaSetting(reinterpret_cast<wchar_t*>(settingName));
            settingsList.append(qMakePair(name, qMakePair(settingIds[i], currentSetting.u32CurrentValue)));
        } else {
            qWarning() << "无法获取设置名称，ID:" << settingIds[i] << "错误码:" << status;
        }
    }

    delete[] settingIds;

    for(auto e : settingsList)
    {
        qDebug()<<e<<Qt::endl;
    }
    return settingsList;
}

NvAPI_Status NVIDIAGraphicsConfigManager::CheckDisplayConfig() {
    NvAPI_Status status = NVAPI_OK;
    NvU32 deviceCount = 0;
    NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo = nullptr;

    // 初始化NVIDIA API
    if (!NVIDIAGraphicsConfigManager::initializeNvAPI()) {
        qWarning().noquote() << QString("[错误] NVIDIA API初始化失败 (0x%1)").arg(status, 0, 16);
        return NVAPI_API_NOT_INITIALIZED;
    }

    // 第一阶段：获取设备数量
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status != NVAPI_OK || deviceCount == 0) {
        qWarning().noquote() << QString("[错误] 无法获取显示设备数量 (0x%1)").arg(status, 0, 16);
        return status ? status : NVAPI_ERROR;
    }
    qDebug().noquote() << QString("\n▶ 检测到 %1 个显示设备").arg(deviceCount);

    // 分配路径信息内存
    pathInfo = new NV_DISPLAYCONFIG_PATH_INFO_V2[deviceCount];
    memset(pathInfo, 0, sizeof(NV_DISPLAYCONFIG_PATH_INFO_V2) * deviceCount);
    for (NvU32 i = 0; i < deviceCount; ++i) {
        pathInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER2;
    }

    // 第二阶段：获取详细配置
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status != NVAPI_OK) {
        qWarning().noquote() << QString("[错误] 无法获取显示配置详情 (0x%1)").arg(status, 0, 16);
        delete[] pathInfo;
        return status;
    }
    qDebug().noquote() << "▶ 成功获取显示配置结构";

    // 初始化嵌套数据结构
    for (NvU32 i = 0; i < deviceCount; ++i) {
        pathInfo[i].sourceModeInfo = new NV_DISPLAYCONFIG_SOURCE_MODE_INFO_V1();
        pathInfo[i].sourceModeInfo->reserved = 0;

        if (pathInfo[i].targetInfoCount > 0) {
            pathInfo[i].targetInfo = new NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2[pathInfo[i].targetInfoCount];
            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                pathInfo[i].targetInfo[j].details = new NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_V1();
                pathInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER1;
            }
        }
    }

    // 第三阶段：验证配置
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status == NVAPI_OK) {
        qDebug().noquote() << "▶ 当前显示配置：";
        for (NvU32 i = 0; i < deviceCount; ++i) {
            qDebug().noquote() << QString("  设备#%1 有 %2 个显示目标").arg(i+1).arg(pathInfo[i].targetInfoCount);
            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                qDebug().noquote() << QString("    → 目标%1: %2")
                                          .arg(j+1)
                                          .arg(DecodeScalingMode(pathInfo[i].targetInfo[j].details->scaling));
            }
        }
    }

    // 配置验证
    status = NvAPI_DISP_SetDisplayConfig(deviceCount, pathInfo, NV_DISPLAYCONFIG_VALIDATE_ONLY);
    if (status != NVAPI_OK) {
        qWarning().noquote() << QString("[警告] 配置验证未通过 (0x%1)").arg(status, 0, 16);
    }

    // 内存清理
    for (NvU32 i = 0; i < deviceCount; ++i) {
        delete pathInfo[i].sourceModeInfo;
        if (pathInfo[i].targetInfo) {
            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                delete pathInfo[i].targetInfo[j].details;
            }
            delete[] pathInfo[i].targetInfo;
        }
    }
    delete[] pathInfo;

    return status;
}

bool NVIDIAGraphicsConfigManager::initializeNvAPI()
{
    if (!isNvAPIInitialized) {
        NvAPI_Status status = NvAPI_Initialize();
        if (status != NVAPI_OK) {
            qWarning() << "NvAPI_Initialize failed with error:" << status;
            return false;
        }
        status = NvAPI_DRS_CreateSession(&globalSession);
        if (status != NVAPI_OK) {
            qWarning() << "NvAPI_DRS_CreateSession failed with error:" << status;
            NvAPI_Unload();
            return false;
        }
        isNvAPIInitialized = true;
    }
    return true;
}

void NVIDIAGraphicsConfigManager::shutdownNvAPI()
{
    if (isNvAPIInitialized) {
        NvAPI_DRS_DestroySession(globalSession);
        NvAPI_Unload();
        isNvAPIInitialized = false;
    }
}

NvU32 NVIDIAGraphicsConfigManager::getSettingId(const wchar_t* settingName) {
    NvU32 settingId = 0;
    QString qstr = QString::fromWCharArray(settingName);
    if (NvAPI_DRS_GetSettingIdFromName((NvU16*)(qstr.utf16()), &settingId) != NVAPI_OK) {
        qWarning() << "Failed to get setting ID for" << QString::fromWCharArray(settingName);
    }
    return settingId;
}

// 新增的通用设置方法
bool NVIDIAGraphicsConfigManager::applyNvidiaSetting(NVDRS_SETTING& drsSetting) {
    if (!initializeNvAPI()) return false;

    NvAPI_Status status = NvAPI_DRS_LoadSettings(globalSession);
    if (status != NVAPI_OK) {
        PrintError(status);
        return false;
    }

    NvDRSProfileHandle hProfile = 0;
    status = NvAPI_DRS_GetCurrentGlobalProfile(globalSession, &hProfile);
    if (status != NVAPI_OK) {
        PrintError(status);
        return false;
    }

    status = NvAPI_DRS_SetSetting(globalSession, hProfile, &drsSetting);
    if (status != NVAPI_OK) {
        PrintError(status);
        return false;
    }

    status = NvAPI_DRS_SaveSettings(globalSession);
    return status == NVAPI_OK;
}

// 新增的通用查询方法
void NVIDIAGraphicsConfigManager::FreeDisplayConfigResources(NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo, NvU32 pathCount) {
    for (NvU32 i = 0; i < pathCount; ++i) {
        delete pathInfo[i].sourceModeInfo;
        if (pathInfo[i].targetInfo) {
            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                delete pathInfo[i].targetInfo[j].details;
            }
            delete[] pathInfo[i].targetInfo;
        }
    }
    delete[] pathInfo;
}

NVDRS_SETTING NVIDIAGraphicsConfigManager::queryNvidiaSetting(const wchar_t* settingName) {
    NVDRS_SETTING setting = {0};

    if (!initializeNvAPI()) {
        return setting;
    }

    // 加载最新配置（确保数据同步）
    NvAPI_Status status = NvAPI_DRS_LoadSettings(globalSession);
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_LoadSettings failed with error:" << status;
        return setting;
    }

    // 优先尝试当前全局配置（而非仅基础配置）
    NvDRSProfileHandle hProfile = 0;
    status = NvAPI_DRS_GetBaseProfile(globalSession, &hProfile);
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_GetCurrentGlobalProfile failed with error:" << status;
        return setting;
    }

    NvU32 settingId = getSettingId(settingName);
    if (settingId == 0) {
        return setting;
    }

    setting.version = NVDRS_SETTING_VER;
    setting.settingId = settingId;
    setting.settingType = NVDRS_DWORD_TYPE;
    status = NvAPI_DRS_GetSetting(globalSession, hProfile, settingId, &setting);
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_GetSetting failed with error:" << status << " string:"<<QString::fromWCharArray(settingName);
    }

    return setting;
}

NvAPI_Status NVIDIAGraphicsConfigManager::AllocateAndGetDisplayConfig(NvU32* pathInfoCount, NV_DISPLAYCONFIG_PATH_INFO** pPathInfo)
{
    NvAPI_Status ret;

    // Retrieve the display path information
    NvU32 pathCount							= 0;
    NV_DISPLAYCONFIG_PATH_INFO *pathInfo	= NULL;

    ret = NvAPI_DISP_GetDisplayConfig(&pathCount, NULL);
    if (ret != NVAPI_OK)    return ret;

    pathInfo = (NV_DISPLAYCONFIG_PATH_INFO*) malloc(pathCount * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
    if (!pathInfo)
    {
        return NVAPI_OUT_OF_MEMORY;
    }

    memset(pathInfo, 0, pathCount * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
    for (NvU32 i = 0; i < pathCount; i++)
    {
        pathInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER;
    }

    // Retrieve the targetInfo counts
    ret = NvAPI_DISP_GetDisplayConfig(&pathCount, pathInfo);
    if (ret != NVAPI_OK)
    {
        return ret;
    }

    for (NvU32 i = 0; i < pathCount; i++)
    {
        // Allocate the source mode info

        if(pathInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER1 || pathInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER2)
        {
            pathInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*) malloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
        }
        else
        {

#ifdef NV_DISPLAYCONFIG_PATH_INFO_VER3
            pathInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*) malloc(pathInfo[i].sourceModeInfoCount * sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
#endif

        }
        if (pathInfo[i].sourceModeInfo == NULL)
        {
            return NVAPI_OUT_OF_MEMORY;
        }
        memset(pathInfo[i].sourceModeInfo, 0, sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));

        // Allocate the target array
        pathInfo[i].targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*) malloc(pathInfo[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
        if (pathInfo[i].targetInfo == NULL)
        {
            return NVAPI_OUT_OF_MEMORY;
        }
        // Allocate the target details
        memset(pathInfo[i].targetInfo, 0, pathInfo[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
        for (NvU32 j = 0 ; j < pathInfo[i].targetInfoCount ; j++)
        {
            pathInfo[i].targetInfo[j].details = (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*) malloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
            memset(pathInfo[i].targetInfo[j].details, 0, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
            pathInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
        }
    }

    // Retrieve the full path info
    ret = NvAPI_DISP_GetDisplayConfig(&pathCount, pathInfo);
    if (ret != NVAPI_OK)
    {
        return ret;
    }

    *pathInfoCount = pathCount;
    *pPathInfo = pathInfo;
    return NVAPI_OK;
}

int NVIDIAGraphicsConfigManager::getVSyncMode() {
    NVDRS_SETTING setting = queryNvidiaSetting(VSYNCMODE_STRING);

    switch(setting.u32CurrentValue)
    {
    case VSYNCMODE_PASSIVE:
    {
        return 1;
    }
    case VSYNCMODE_FORCEOFF:
    {
        return 2;
    }
    case VSYNCMODE_FORCEON:
    {
        return 3;
    }
    case VSYNCMODE_FLIPINTERVAL2:
    {
        return 4;
    }
    case VSYNCMODE_FLIPINTERVAL3:
    {
        return 5;
    }
    case VSYNCMODE_FLIPINTERVAL4:
    {
        return 6;
    }
    case VSYNCMODE_VIRTUAL:
    {
        return 7;
    }
    case VSYNCMODE_NUM_VALUES:
    {
        return 8;
    }
    default:
        return 0;
    }
}

void NVIDIAGraphicsConfigManager::setVSyncMode(int mode) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(VSYNCMODE_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;

    switch(mode)//可能后续还有其它操作，先这样写
    {
        case 1:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_PASSIVE;
            break;
        }
        case 2:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_FORCEOFF;
            break;
        }
        case 3:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_FORCEON;
            break;
        }
        case 4:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_FLIPINTERVAL2;
            break;
        }
        case 5:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_FLIPINTERVAL3;
            break;
        }
        case 6:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_FLIPINTERVAL4;
            break;
        }
        case 7:
        {
            drsSetting.u32CurrentValue = VSYNCMODE_VIRTUAL;
            break;
        }
        default:
        {
            emit errorOccurred(tr("应用垂直同步设置失败，因为这个设置不认识"));
            return ;
        }
    }

    if(drsSetting.settingId!=0 && drsSetting.u32CurrentValue!=0)
    {
        if (!applyNvidiaSetting(drsSetting)) {
            emit errorOccurred(tr("应用垂直同步设置失败"));
        }
        emit errorOccurred(tr("应用垂直同步设置成功"));
    }
    emit vSyncModeChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getImageSharpeningStatus() {
    //getAllNvidiaSettings();
    NVDRS_SETTING setting = queryNvidiaSetting(NV_QUALITY_UPSCALING_STRING);

    switch(setting.u32CurrentValue)
    {
    case NV_QUALITY_UPSCALING_OFF:
    {
        return 1;
    }
    case NV_QUALITY_UPSCALING_ON:
    {
        return 2;
    }
    default:
        return 0;
    }
}

void NVIDIAGraphicsConfigManager::setImageSharpening(int index) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(NV_QUALITY_UPSCALING_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;


    switch(index)
    {
    case 1:
    {
        drsSetting.u32CurrentValue = NV_QUALITY_UPSCALING_OFF;
        break;
    }
    case 2:
    {
        drsSetting.u32CurrentValue = NV_QUALITY_UPSCALING_ON;
        break;
    }
    default:
    {
        emit errorOccurred(tr("应用图形增强设置失败，因为这个设置不认识"));
        return ;
    }
    }

    if (!applyNvidiaSetting(drsSetting)) {
        emit errorOccurred(tr("应用图形增强设置失败"));
    }
    emit imageSharpeningChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getOpenGLGDICompatibility() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_CPL_GDI_COMPATIBILITY_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_CPL_GDI_COMPATIBILITY_PREFER_DISABLED: return 1;
        case OGL_CPL_GDI_COMPATIBILITY_PREFER_ENABLED: return 2;
        case OGL_CPL_GDI_COMPATIBILITY_AUTO: return 3;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setOpenGLGDICompatibility(int mode) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(OGL_CPL_GDI_COMPATIBILITY_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;

    switch(mode) {
        case 1: drsSetting.u32CurrentValue = OGL_CPL_GDI_COMPATIBILITY_PREFER_DISABLED; break;
        case 2: drsSetting.u32CurrentValue = OGL_CPL_GDI_COMPATIBILITY_PREFER_ENABLED; break;
        case 3: drsSetting.u32CurrentValue = OGL_CPL_GDI_COMPATIBILITY_AUTO; break;
        default:
            emit errorOccurred(tr("不支持的GDI兼容模式"));
            return;
    }

    if(!applyNvidiaSetting(drsSetting)) {
        emit errorOccurred(tr("应用GDI兼容设置失败"));
    }
    emit openGLGDICompatibilityChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getOpenGLPresentMethod() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_CPL_PREFER_DXPRESENT_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_CPL_PREFER_DXPRESENT_PREFER_DISABLED: return 1;
        case OGL_CPL_PREFER_DXPRESENT_PREFER_ENABLED: return 2;
        case OGL_CPL_PREFER_DXPRESENT_AUTO: return 3;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setOpenGLPresentMethod(int mode) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(OGL_CPL_PREFER_DXPRESENT_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;

    switch(mode) {
        case 1: drsSetting.u32CurrentValue = OGL_CPL_PREFER_DXPRESENT_PREFER_DISABLED; break;
        case 2: drsSetting.u32CurrentValue = OGL_CPL_PREFER_DXPRESENT_PREFER_ENABLED; break;
        case 3: drsSetting.u32CurrentValue = OGL_CPL_PREFER_DXPRESENT_AUTO; break;
        default:
            emit errorOccurred(tr("不支持的呈现模式"));
            return;
    }

    if(!applyNvidiaSetting(drsSetting)) {
        emit errorOccurred(tr("应用呈现方法设置失败"));
    }
    emit openGLPresentMethodChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getTripleBuffer() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_TRIPLE_BUFFER_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_TRIPLE_BUFFER_DISABLED: return 1;
        case OGL_TRIPLE_BUFFER_ENABLED: return 2;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setTripleBuffer(int mode) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(OGL_TRIPLE_BUFFER_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;

    switch(mode) {
        case 1: drsSetting.u32CurrentValue = OGL_TRIPLE_BUFFER_DISABLED; break;
        case 2: drsSetting.u32CurrentValue = OGL_TRIPLE_BUFFER_ENABLED; break;
        default:
            emit errorOccurred(tr("不支持的三重缓冲模式"));
            return;
    }

    if(!applyNvidiaSetting(drsSetting)) {
        emit errorOccurred(tr("应用三重缓冲设置失败"));
    }
    emit tripleBufferChanged();//触发信号，让qml更新u
}


int NVIDIAGraphicsConfigManager::getPowerManagementMode() {
    NVDRS_SETTING setting = queryNvidiaSetting(PREFERRED_PSTATE_STRING);

    switch(setting.u32CurrentValue) {
    case PREFERRED_PSTATE_ADAPTIVE: return 1;
    case PREFERRED_PSTATE_PREFER_MAX: return 2;
    case PREFERRED_PSTATE_DRIVER_CONTROLLED: return 3;
    case PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE: return 4;
    case PREFERRED_PSTATE_PREFER_MIN: return 5;
    case PREFERRED_PSTATE_OPTIMAL_POWER: return 6;
    default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setPowerManagementMode(int mode) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(PREFERRED_PSTATE_STRING);
    drsSetting.settingType = NVDRS_DWORD_TYPE;

    switch(mode) {
        case 1: drsSetting.u32CurrentValue = PREFERRED_PSTATE_ADAPTIVE; break;
        case 2: drsSetting.u32CurrentValue = PREFERRED_PSTATE_PREFER_MAX; break;
        case 3: drsSetting.u32CurrentValue = PREFERRED_PSTATE_DRIVER_CONTROLLED; break;
        case 4: drsSetting.u32CurrentValue = PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE; break;
        case 5: drsSetting.u32CurrentValue = PREFERRED_PSTATE_PREFER_MIN; break;
        case 6: drsSetting.u32CurrentValue = PREFERRED_PSTATE_OPTIMAL_POWER; break;
        default:
            emit errorOccurred(tr("无效的电源管理模式选项"));
            return;
    }

    if (!applyNvidiaSetting(drsSetting)) {
        emit errorOccurred(tr("应用电源管理模式失败"));
    }
    emit powerManagementModeChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getAppIdleFPSLimit() {
    NVDRS_SETTING setting = queryNvidiaSetting(APPIDLE_DYNAMIC_FRL_FPS_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setAppIdleFPSLimit(int fps) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(APPIDLE_DYNAMIC_FRL_FPS_STRING); // 确保这是正确的ID，可能需要根据实际情况调整，或者使用NVDRS_SETTING_ID_APPIDLE_DYNAMIC_FRL_FPS;
    drsSetting.settingType = NVDRS_DWORD_TYPE;
    drsSetting.u32CurrentValue = qBound(0, fps, 0x3ff);
    if(!applyNvidiaSetting(drsSetting)) {
        //emit appIdleFPSLimitChanged();
        emit errorOccurred(tr("应用应用空闲时的FPS限制失败"));
    }
    emit appIdleFPSLimitChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getAnisotropicFiltering() {
    NVDRS_SETTING setting = queryNvidiaSetting(ANISO_MODE_LEVEL_STRING);
    return (setting.u32CurrentValue & ANISO_MODE_LEVEL_MASK) - 1;//qml array组件只能从0开始计数
}

void NVIDIAGraphicsConfigManager::setAnisotropicFiltering(int level) {
    level +=1;//qml array组件只能从0开始计数

    {
        NVDRS_SETTING drsSetting = {0};
        drsSetting.version = NVDRS_SETTING_VER;
        drsSetting.settingId = ANISO_MODE_SELECTOR_ID;
        drsSetting.settingType = NVDRS_DWORD_TYPE;
        if(level == 1)//1x是关闭选项，大于1x要开启这2个选项，等于1x就是关闭这2个选项
        {
            drsSetting.u32CurrentValue = 0;
        }
        else
        {
            drsSetting.u32CurrentValue = 1;
        }
        if(!applyNvidiaSetting(drsSetting)) {
            // 属性赋值会自动触发anisotropicFilteringChanged信号
            emit errorOccurred(tr("异性过滤设置失败"));
            return ;
        }
    }

    {
        NVDRS_SETTING drsSetting = {0};
        drsSetting.version = NVDRS_SETTING_VER;
        drsSetting.settingId = PS_TEXFILTER_NO_NEG_LODBIAS_ID;
        drsSetting.settingType = NVDRS_DWORD_TYPE;
        if(level == 1)//1x是关闭选项，大于1x要开启这2个选项，等于1x就是关闭这2个选项
        {
            drsSetting.u32CurrentValue = 0;
        }
        else
        {
            drsSetting.u32CurrentValue = 1;
        }

        if(!applyNvidiaSetting(drsSetting)) {
            // 属性赋值会自动触发anisotropicFilteringChanged信号
            emit errorOccurred(tr("异性过滤设置失败"));
            return ;
        }
    }

    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = ANISO_MODE_LEVEL_ID;
    drsSetting.settingType = NVDRS_DWORD_TYPE;
    drsSetting.u32CurrentValue = level;
    
    if(!applyNvidiaSetting(drsSetting)) {
        // 属性赋值会自动触发anisotropicFilteringChanged信号
        emit errorOccurred(tr("异性过滤设置失败"));
    }
    emit anisotropicFilteringChanged();//触发信号，让qml更新u
}

int NVIDIAGraphicsConfigManager::getAAModeSelector() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_SELECTOR_STRING);
    switch(setting.u32CurrentValue) {
        case AA_MODE_SELECTOR_APP_CONTROL: return 0;
        case AA_MODE_SELECTOR_OVERRIDE: return 1;
        case AA_MODE_SELECTOR_ENHANCE: return 2;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setAAModeSelector(int mode) {
    {
        if(mode == 2 || mode == 3)//如果是“提高应用程序设置”或"置换任何应用程序设置"
        {
            setAAModeMethod(1);
        }
        else
        {
            setAAModeMethod(0);
        }
    }

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = AA_MODE_SELECTOR_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    
    switch(mode) {
        case 0: setting.u32CurrentValue = AA_MODE_SELECTOR_APP_CONTROL; break;
        case 1: setting.u32CurrentValue = AA_MODE_SELECTOR_OVERRIDE; break;
        case 2: setting.u32CurrentValue = AA_MODE_SELECTOR_ENHANCE; break;
        case 3: setting.u32CurrentValue = AA_MODE_SELECTOR_OVERRIDE; break;
        default: return;
    }
    
    if(!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("抗锯齿模式选择设置失败"));
    }
    emit aaModeSelectorChanged();
}

int NVIDIAGraphicsConfigManager::getAAModeMethod() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_METHOD_STRING);
    
    switch(setting.u32CurrentValue) {
        case AA_MODE_METHOD_NONE: return 0;
        case AA_MODE_METHOD_MULTISAMPLE_2X_DIAGONAL: return 1;
        case AA_MODE_METHOD_MULTISAMPLE_4X: return 2;
        case AA_MODE_METHOD_MULTISAMPLE_8X: return 3;
        case AA_MODE_METHOD_MULTISAMPLE_16X: return 4;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setAAModeMethod(int mode) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = AA_MODE_METHOD_ID;
    setting.settingType = NVDRS_DWORD_TYPE;

    switch(mode) {
        case 0: setting.u32CurrentValue = AA_MODE_METHOD_NONE; break;
        case 1: setting.u32CurrentValue = AA_MODE_METHOD_MULTISAMPLE_2X_DIAGONAL; break;
        case 2: setting.u32CurrentValue = AA_MODE_METHOD_MULTISAMPLE_4X; break;
        case 3: setting.u32CurrentValue = AA_MODE_METHOD_MULTISAMPLE_8X; break;
        case 4: setting.u32CurrentValue = AA_MODE_METHOD_MULTISAMPLE_16X; break;
        default: return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("抗锯齿模式设置失败");
    } else {
        emit aaModeMethodChanged();
    }
    emit aaModeMethodChanged();
}

int NVIDIAGraphicsConfigManager::getFXAAEnable() {
    NVDRS_SETTING setting = queryNvidiaSetting(FXAA_ENABLE_STRING);
    switch(setting.u32CurrentValue) {
        case FXAA_ENABLE_OFF: return 1;
        case FXAA_ENABLE_ON:  return 2;
        default:              return 0;
    }
}

void NVIDIAGraphicsConfigManager::setFXAAEnable(int index) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = getSettingId(FXAA_ENABLE_STRING);
    setting.settingType = NVDRS_DWORD_TYPE;

    switch(index) {
        case 1: setting.u32CurrentValue = FXAA_ENABLE_OFF; break;
        case 2: setting.u32CurrentValue = FXAA_ENABLE_ON;  break;
        default:
            emit errorOccurred(tr("未知的FXAA设置选项"));
            return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("FXAA设置失败"));
    }
    emit fxaaEnableChanged();
}


int NVIDIAGraphicsConfigManager::getAAGammaCorrection() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_GAMMACORRECTION_STRING);
    return setting.u32CurrentValue; // 直接返回原始值
}

void NVIDIAGraphicsConfigManager::setAAGammaCorrection(int level) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = getSettingId(AA_MODE_GAMMACORRECTION_STRING);
    setting.settingType = NVDRS_DWORD_TYPE;
    
    if(level < 0 || level > 2) {
        emit errorOccurred("无效的伽马校正选项，有效范围0-2");
        return;
    }
    
    setting.u32CurrentValue = level;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("抗锯齿伽马校正设置失败");
    }
    emit aaGammaCorrectionChanged();
}

int NVIDIAGraphicsConfigManager::getAATransparency() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_REPLAY_STRING);//英伟达控制面板查到的是0 0 20 36 52
    return setting.u32CurrentValue & 0x00000070;
}

void NVIDIAGraphicsConfigManager::setAATransparency(int mode) {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_REPLAY_STRING);
    
    {
        NVDRS_SETTING setting = {};
        setting.version = NVDRS_SETTING_VER;
        setting.settingId = AA_MODE_ALPHATOCOVERAGE_ID;
        setting.settingType = NVDRS_DWORD_TYPE;
        if(mode == 1)//如果是“多重取样”选项
            setting.u32CurrentValue = AA_MODE_ALPHATOCOVERAGE_MODE_ON;
        else
            setting.u32CurrentValue = AA_MODE_ALPHATOCOVERAGE_MODE_OFF;
        if(!applyNvidiaSetting(setting)) {
            emit errorOccurred(tr("抗锯齿模式选择设置失败"));
        }
    }

    switch(mode) {
        case 0: setting.u32CurrentValue = 0; break;
        case 1: setting.u32CurrentValue = 0; break;
        case 2: setting.u32CurrentValue = AA_MODE_REPLAY_SAMPLES_FOUR; break;//"2x(超级取样)"
        case 3: setting.u32CurrentValue = 36; break;//"4x(超级取样)"enum EValues_AA_MODE_REPLAY 没有这个数值，可能是版本不对，这可能是一个废弃的值，我的驱动版本：32.0.15.6094，英伟达控制面板版本：8.1.967.0，开发SDK：R560
        case 4: setting.u32CurrentValue = 52; break;//"8x(超级取样)"也没有这个值
        default: return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("透明抗锯齿设置失败，请检查驱动支持"));
    }
    emit aaTransparencyChanged();
}


int NVIDIAGraphicsConfigManager::getLowLatencyMode() {
    NVDRS_SETTING setting = queryNvidiaSetting(PRERENDERLIMIT_STRING);
    int index = 0;
    switch(setting.u32CurrentValue)
    {
    case 0:
    {
        index = 1;
        break;
    }
    case 1:
    {
        index = 2;
        break;
    }
    default :
        index = 0;
    }

    return index;
}

void NVIDIAGraphicsConfigManager::setLowLatencyMode(int value) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PRERENDERLIMIT_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    switch(value)
    {
    case 1:setting.u32CurrentValue = 0;break;
    case 2:setting.u32CurrentValue = 1;break;
    default:
        emit errorOccurred(tr("未知的选项，设置低延迟模式失败"));
        return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("设置低延迟模式失败"));
    }
    emit lowLatencyModeChanged();
}




int NVIDIAGraphicsConfigManager::getMaxFPSLimit() {
    NVDRS_SETTING setting = queryNvidiaSetting(FRL_FPS_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setMaxFPSLimit(int limit) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = FRL_FPS_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = qBound(0, limit, 0x3ff);

    if(!applyNvidiaSetting(setting)) {
        emit errorOccurred("最大帧率限制设置失败");
    }
    emit maxFPSLimitChanged();
}

int NVIDIAGraphicsConfigManager::getAoMode()  {
    NVDRS_SETTING setting = queryNvidiaSetting(AO_MODE_STRING);
    switch(setting.u32CurrentValue) {
        case AO_MODE_OFF: return 0;
        case AO_MODE_LOW: return 1;
        case AO_MODE_MEDIUM: return 2;
        case AO_MODE_HIGH: return 3;
        default: return 0;
    }
}

void NVIDIAGraphicsConfigManager::setAoMode(int value) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = AO_MODE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    switch(value) {
       case 0: setting.u32CurrentValue = AO_MODE_OFF; break;
       case 1: setting.u32CurrentValue = AO_MODE_LOW; break;
       case 2: setting.u32CurrentValue = AO_MODE_MEDIUM; break;
       case 3: setting.u32CurrentValue = AO_MODE_HIGH; break; 
    }
    if(!applyNvidiaSetting(setting))
    {
        emit errorOccurred(tr("设置环境光模式失败"));
    }
    emit aoModeChanged();//get set不在同一个控件里，得自己触发
}


int NVIDIAGraphicsConfigManager::getShaderCacheSize() {
    NVDRS_SETTING setting = queryNvidiaSetting(PS_SHADERDISKCACHE_MAX_SIZE_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setShaderCacheSize(int value) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PS_SHADERDISKCACHE_MAX_SIZE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = value;

    if(!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("设置着色器缓存大小失败"));
        return;
    }
    emit shaderCacheSizeChanged();//get set不在同一个控件里，得自己触发
}

int NVIDIAGraphicsConfigManager::getTrilinearOptimization()
{
    NVDRS_SETTING setting = queryNvidiaSetting(PS_TEXFILTER_DISABLE_TRILIN_SLOPE_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setTrilinearOptimization(int mode)
{
    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PS_TEXFILTER_DISABLE_TRILIN_SLOPE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = mode;//英伟达控制面板里面，关是1，开是0,貌似和定义相反的

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("Failed to set trilinear optimization");
        return;
    }
    emit trilinearOptimizationChanged();
}

int NVIDIAGraphicsConfigManager::getAnisotropicSampleOptimization() {
    NVDRS_SETTING setting = queryNvidiaSetting(PS_TEXFILTER_ANISO_OPTS2_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setAnisotropicSampleOptimization(int mode) {
    NVDRS_SETTING setting;
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PS_TEXFILTER_ANISO_OPTS2_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = mode;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("设置各向异性采样优化失败");
    } else {
        emit anisotropicSampleOptimizationChanged();
    }
}

int NVIDIAGraphicsConfigManager::getNegativeLODBias() {
    NVDRS_SETTING setting = queryNvidiaSetting(PS_TEXFILTER_NO_NEG_LODBIAS_STRING);
    return setting.u32CurrentValue;
}

void NVIDIAGraphicsConfigManager::setNegativeLODBias(int mode) {
    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PS_TEXFILTER_NO_NEG_LODBIAS_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = mode;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("设置负LOD偏置失败");
    } else {
        emit negativeLODBiasChanged();
    }
}

int NVIDIAGraphicsConfigManager::getTextureFilterQuality() {
    NVDRS_SETTING setting = queryNvidiaSetting(QUALITY_ENHANCEMENTS_STRING);

    switch(setting.u32CurrentValue)
    {
    case QUALITY_ENHANCEMENTS_HIGHQUALITY:
        return 0;
    case QUALITY_ENHANCEMENTS_QUALITY:
        return 1;
    case QUALITY_ENHANCEMENTS_PERFORMANCE:
        return 2;
    case QUALITY_ENHANCEMENTS_HIGHPERFORMANCE:
        return 3;
    default:
        return 1;
    }
}

void NVIDIAGraphicsConfigManager::setTextureFilterQuality(int quality) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = QUALITY_ENHANCEMENTS_ID;
    setting.settingType = NVDRS_DWORD_TYPE;

    switch(quality)
    {
    case 0:
        setting.u32CurrentValue = QUALITY_ENHANCEMENTS_HIGHQUALITY;
        break;
    case 1:
        setting.u32CurrentValue = QUALITY_ENHANCEMENTS_QUALITY;
        break;
    case 2:
        setting.u32CurrentValue = QUALITY_ENHANCEMENTS_PERFORMANCE;
        break;
    case 3:
        setting.u32CurrentValue = QUALITY_ENHANCEMENTS_HIGHPERFORMANCE;
        break;
    default:
        return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(QStringLiteral("设置纹理过滤质量失败"));
    } else {
        emit textureFilterQualityChanged();
    }
}

int NVIDIAGraphicsConfigManager::getThreadControl() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_THREAD_CONTROL_STRING);

    switch(setting.u32CurrentValue)
    {
    case 0:
        return 0;
    case OGL_THREAD_CONTROL_ENABLE:
        return 1;
    case OGL_THREAD_CONTROL_DISABLE:
        return 2;
    default:
        return 0;
    }
}

void NVIDIAGraphicsConfigManager::setThreadControl(int mode) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = OGL_THREAD_CONTROL_ID;
    setting.settingType = NVDRS_DWORD_TYPE;

    switch(mode)
    {
    case 0:
        setting.u32CurrentValue = 0;
        break;
    case 1:
        setting.u32CurrentValue = OGL_THREAD_CONTROL_ENABLE;
        break;
    case 2:
        setting.u32CurrentValue = OGL_THREAD_CONTROL_DISABLE;
        break;
    default:
        return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(QStringLiteral("设置线程优化失败"));
    } else {
        emit threadControlChanged();
    }
}

const QString NVIDIAGraphicsConfigManager::DecodeScalingMode(NvU32 scaling) {
    switch(scaling) {
    case NV_SCALING_DEFAULT:
        return "默认缩放模式";
    case NV_SCALING_GPU_SCALING_TO_CLOSEST:
        return "GPU缩放至最近分辨率";
    case NV_SCALING_GPU_SCALING_TO_NATIVE:
        return "GPU缩放至原生分辨率";
    case NV_SCALING_GPU_SCANOUT_TO_NATIVE:
        return "GPU直出原生分辨率";
    case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE:
        return "GPU保持宽高比缩放至原生";
    case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST:
        return "GPU保持宽高比缩放至最近";
    default:
        return "未知缩放模式";
    }
}

QByteArray NVIDIAGraphicsConfigManager::getScalingMode() {
    QJsonObject root;
    NvAPI_Status status = NVAPI_OK;
    NvU32 deviceCount = 0;
    NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo = nullptr;

    // 初始化NVIDIA API
    if (!initializeNvAPI()) {
        root["error"] = QJsonValue("NVIDIA_API_INIT_FAILED");
    root["devices"] = QJsonArray();
    return QJsonDocument(root).toJson();
    }

    // 获取设备信息
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status != NVAPI_OK || deviceCount == 0) {
        root["error"] = QJsonValue(QString("DEVICE_COUNT_ERROR_0x%1").arg(status, 0, 16));
    root["devices"] = QJsonArray();
    return QJsonDocument(root).toJson();
    }

    // 构建设备树结构
    QJsonArray devicesArray;
    pathInfo = new NV_DISPLAYCONFIG_PATH_INFO_V2[deviceCount];
    memset(pathInfo, 0, sizeof(NV_DISPLAYCONFIG_PATH_INFO_V2) * deviceCount);
    for (NvU32 i = 0; i < deviceCount; ++i) {
        pathInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER2;
    }

    // 第二阶段：获取详细配置
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status != NVAPI_OK) {
        qWarning().noquote() << QString("[错误] 无法获取显示配置详情 (0x%1)").arg(status, 0, 16);
        delete[] pathInfo;
        root["error"] = QJsonValue(QString("DISPLAY_CONFIG_ERROR_0x%1").arg(status, 0, 16));
    root["devices"] = QJsonArray();
    return QJsonDocument(root).toJson();
    }

    // 初始化嵌套数据结构
    for (NvU32 i = 0; i < deviceCount; ++i) {
        pathInfo[i].sourceModeInfo = new NV_DISPLAYCONFIG_SOURCE_MODE_INFO_V1();
        pathInfo[i].sourceModeInfo->reserved = 0;

        if (pathInfo[i].targetInfoCount > 0) {
            pathInfo[i].targetInfo = new NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2[pathInfo[i].targetInfoCount];
            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                pathInfo[i].targetInfo[j].details = new NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_V1();//这是一个描述显示目标详细配置的结构体指针
                pathInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER1;
            }
        }
    }

    // 第三阶段：验证配置
    status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
    if (status == NVAPI_OK) {
        for (NvU32 i = 0; i < deviceCount; ++i) {
            QJsonObject device;
            QJsonArray targets;

            for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
                QJsonObject target;
                target["displayId"] = static_cast<int>(pathInfo[i].targetInfo[j].displayId);
                target["scaling"] = static_cast<int>(pathInfo[i].targetInfo[j].details->scaling);
                targets.append(QJsonValue(target));
            }

            QJsonObject deviceObj;
            deviceObj["targets"] = targets;
            devicesArray.append(deviceObj);
        }
    }
    // 配置验证
    status = NvAPI_DISP_SetDisplayConfig(deviceCount, pathInfo, NV_DISPLAYCONFIG_VALIDATE_ONLY);
    if (status != NVAPI_OK) {
        qWarning().noquote() << QString("[警告] 配置验证未通过 (0x%1)").arg(status, 0, 16);
    }

    // 内存清理
    FreeDisplayConfigResources(pathInfo, deviceCount);

    root["devices"] = devicesArray;
    return QJsonDocument(root).toJson();
}

void NVIDIAGraphicsConfigManager::setScalingMode(const QByteArray& params)
{
    QByteArray decodedParams = QByteArray::fromBase64(params);
    QJsonDocument doc = QJsonDocument::fromJson(decodedParams);
    if (!doc.isObject()) {
        qWarning() << "Invalid scaling mode parameters:" << decodedParams;
        return;
    }
    QJsonObject obj = doc.object();
    int displayId = obj["displayId"].toInt();
    int mode = obj["mode"].toInt();
    if (!initializeNvAPI()) {
        qWarning() << "NVIDIA API 初始化失败";
        return;
    }

    NvU32 pathCount = 0;
    NV_DISPLAYCONFIG_PATH_INFO* pathInfo = nullptr;
    NvAPI_Status status = AllocateAndGetDisplayConfig(&pathCount, &pathInfo);
    if (status != NVAPI_OK) {
        qWarning() << "获取显示配置失败，错误码:" << status;
        return;
    }

    // 查找匹配的displayId
    for (NvU32 i = 0; i < pathCount; ++i) {
        for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; ++j) {
            if (pathInfo[i].targetInfo[j].displayId == displayId) {//隐式转换不用管
#if 0
                // 创建只包含目标显示器的配置
                NV_DISPLAYCONFIG_PATH_INFO singlePathInfo;
                memcpy(&singlePathInfo, &pathInfo[i], sizeof(NV_DISPLAYCONFIG_PATH_INFO));
                
                // 为targetInfo分配新内存并深度拷贝
                singlePathInfo.targetInfo = new NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2[1];
                memcpy(singlePathInfo.targetInfo, &pathInfo[i].targetInfo[j], sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2));
                
                // 为details分配新内存并深度拷贝
                singlePathInfo.targetInfo[0].details = new NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_V1();
                memcpy(singlePathInfo.targetInfo[0].details, pathInfo[i].targetInfo[j].details, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_V1));
                
                // 只在拷贝后的结构体上设置缩放模式
                singlePathInfo.targetInfo[0].details->scaling = (NV_SCALING)mode;
                singlePathInfo.targetInfoCount = 1;

                //释放内存
                delete singlePathInfo.targetInfo[0].details;
                delete[] singlePathInfo.targetInfo;

                // 应用配置
                status = NvAPI_DISP_SetDisplayConfig(1, &singlePathInfo, NV_DISPLAYCONFIG_SAVE_TO_PERSISTENCE);
#else \
                // 直接在原pathInfo数组上设置缩放模式
                pathInfo[i].targetInfo[j].details->scaling = (NV_SCALING)mode;

                status = NvAPI_DISP_SetDisplayConfig(pathCount, pathInfo, NV_DISPLAYCONFIG_SAVE_TO_PERSISTENCE);
#endif
                if (status != NVAPI_OK) {
                    qWarning() << "设置缩放模式失败，错误码:" << status;
                }
                break;
            }
        }
    }

    // 释放内存
    FreeDisplayConfigResources(pathInfo, pathCount);
    emit scalingModeChanged();
}

