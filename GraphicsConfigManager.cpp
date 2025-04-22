#include "GraphicsConfigManager.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>

#include "NvApiDriverSettings.h"

// NVAPI 字符串长度定义
#define NVAPI_SHORT_STRING_LENGTH 64
#define NVAPI_BINARY_DATA_MAX_SIZE 4096

// 静态成员初始化
bool GraphicsConfigManager::isNvAPIInitialized = false;
NvDRSSessionHandle GraphicsConfigManager::globalSession = 0;

void GraphicsConfigManager::PrintError(NvAPI_Status status)
{
    NvAPI_ShortString szDesc = {0};
    NvAPI_GetErrorMessage(status, szDesc);
    printf(" NVAPI error: %s\n", szDesc);
}

QList<QPair<QString, NvU32>> GraphicsConfigManager::getAllNvidiaSettings() {
    QList<QPair<QString, NvU32>> settingsList;

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
        NvAPI_UnicodeString  settingName = {0};
        status = NvAPI_DRS_GetSettingNameFromId(settingIds[i], &settingName);
        if (status == NVAPI_OK) {
            QString name = QString::fromWCharArray(reinterpret_cast<wchar_t*>(settingName));
            settingsList.append(qMakePair(name, settingIds[i]));
        } else {
            qWarning() << "无法获取设置名称，ID:" << settingIds[i] << "错误码:" << status;
        }
    }

    delete[] settingIds;

    // for(auto e : settingsList)
    // {
    //     qDebug()<<e<<Qt::endl;
    // }
    return settingsList;
}

bool GraphicsConfigManager::initializeNvAPI()
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

void GraphicsConfigManager::shutdownNvAPI()
{
    if (isNvAPIInitialized) {
        NvAPI_DRS_DestroySession(globalSession);
        NvAPI_Unload();
        isNvAPIInitialized = false;
    }
}

NvU32 GraphicsConfigManager::getSettingId(const wchar_t* settingName) {
    NvU32 settingId = 0;
    QString qstr = QString::fromWCharArray(settingName);
    if (NvAPI_DRS_GetSettingIdFromName((NvU16*)(qstr.utf16()), &settingId) != NVAPI_OK) {
        qWarning() << "Failed to get setting ID for" << QString::fromWCharArray(settingName);
    }
    return settingId;
}

// 新增的通用设置方法
bool GraphicsConfigManager::applyNvidiaSetting(NVDRS_SETTING& drsSetting) {
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
NVDRS_SETTING GraphicsConfigManager::queryNvidiaSetting(const wchar_t* settingName) {
    NVDRS_SETTING setting = {0};

    if (!initializeNvAPI()) {
        return setting;
    }

    // 加载最新配置（确保数据同步）
    NvAPI_Status status = NvAPI_DRS_LoadSettings(globalSession);
    qDebug() << "配置加载状态:" << status << "会话句柄:" << globalSession;
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_LoadSettings failed with error:" << status;
        return setting;
    }

    // 优先尝试当前全局配置（而非仅基础配置）
    NvDRSProfileHandle hProfile = 0;
    status = NvAPI_DRS_GetBaseProfile(globalSession, &hProfile);
    qDebug() << "当前全局配置文件句柄:" << hProfile << "状态码:" << status;
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
    qDebug() << "NvAPI_DRS_GetSetting返回状态:" << status << "配置文件句柄:" << hProfile;
    if (status != NVAPI_OK) {
        qWarning() << "NvAPI_DRS_GetSetting failed with error:" << status;
    }

    return setting;
}

int GraphicsConfigManager::getVSyncMode() {
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

void GraphicsConfigManager::setVSyncMode(int mode) {
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
}

int GraphicsConfigManager::getImageSharpeningStatus() {
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

void GraphicsConfigManager::setImageSharpening(int index) {
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
}

int GraphicsConfigManager::getOpenGLGDICompatibility() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_CPL_GDI_COMPATIBILITY_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_CPL_GDI_COMPATIBILITY_PREFER_DISABLED: return 1;
        case OGL_CPL_GDI_COMPATIBILITY_PREFER_ENABLED: return 2;
        case OGL_CPL_GDI_COMPATIBILITY_AUTO: return 3;
        default: return 0;
    }
}

void GraphicsConfigManager::setOpenGLGDICompatibility(int mode) {
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
}

int GraphicsConfigManager::getOpenGLPresentMethod() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_CPL_PREFER_DXPRESENT_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_CPL_PREFER_DXPRESENT_PREFER_DISABLED: return 1;
        case OGL_CPL_PREFER_DXPRESENT_PREFER_ENABLED: return 2;
        case OGL_CPL_PREFER_DXPRESENT_AUTO: return 3;
        default: return 0;
    }
}

void GraphicsConfigManager::setOpenGLPresentMethod(int mode) {
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
}

int GraphicsConfigManager::getTripleBuffer() {
    NVDRS_SETTING setting = queryNvidiaSetting(OGL_TRIPLE_BUFFER_STRING);
    switch(setting.u32CurrentValue) {
        case OGL_TRIPLE_BUFFER_DISABLED: return 1;
        case OGL_TRIPLE_BUFFER_ENABLED: return 2;
        default: return 0;
    }
}

void GraphicsConfigManager::setTripleBuffer(int mode) {
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
}


int GraphicsConfigManager::getPowerManagementMode() {
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

void GraphicsConfigManager::setPowerManagementMode(int mode) {
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
}

int GraphicsConfigManager::getAppIdleFPSLimit() {
    NVDRS_SETTING setting = queryNvidiaSetting(APPIDLE_DYNAMIC_FRL_FPS_STRING);
    return setting.u32CurrentValue;
}

void GraphicsConfigManager::setAppIdleFPSLimit(int fps) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = getSettingId(APPIDLE_DYNAMIC_FRL_FPS_STRING); // 确保这是正确的ID，可能需要根据实际情况调整，或者使用NVDRS_SETTING_ID_APPIDLE_DYNAMIC_FRL_FPS;
    drsSetting.settingType = NVDRS_DWORD_TYPE;
    drsSetting.u32CurrentValue = qBound(0, fps, 240);
    if(!applyNvidiaSetting(drsSetting)) {
        //emit appIdleFPSLimitChanged();
        emit errorOccurred(tr("应用应用空闲时的FPS限制失败"));
    }
}

int GraphicsConfigManager::getAnisotropicFiltering() {
    NVDRS_SETTING setting = queryNvidiaSetting(ANISO_MODE_LEVEL_STRING);
    return setting.u32CurrentValue & ANISO_MODE_LEVEL_MASK;
}

void GraphicsConfigManager::setAnisotropicFiltering(int level) {
    NVDRS_SETTING drsSetting = {0};
    drsSetting.version = NVDRS_SETTING_VER;
    drsSetting.settingId = ANISO_MODE_LEVEL_ID;
    drsSetting.settingType = NVDRS_DWORD_TYPE;
    drsSetting.u32CurrentValue = level;
    
    if(applyNvidiaSetting(drsSetting)) {
        // 属性赋值会自动触发anisotropicFilteringChanged信号
        emit errorOccurred(tr("异性过滤设置失败"));
    }
}

int GraphicsConfigManager::getAAModeMethod() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_METHOD_STRING);
    return setting.u32CurrentValue == AA_MODE_METHOD_MULTISAMPLE_4X ? 1 : 0;
}

void GraphicsConfigManager::setAAModeMethod(int mode) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = getSettingId(AA_MODE_METHOD_STRING);
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = mode > 0 ? AA_MODE_METHOD_MULTISAMPLE_4X : AA_MODE_METHOD_NONE;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("抗锯齿模式设置失败");
    } else {
        emit aaModeMethodChanged();
    }
}

bool GraphicsConfigManager::getFXAAEnable() {
    NVDRS_SETTING setting = queryNvidiaSetting(FXAA_ENABLE_STRING);
    return setting.u32CurrentValue != 0;
}

void GraphicsConfigManager::setFXAAEnable(bool enable) {
    NVDRS_SETTING setting = {0};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = getSettingId(FXAA_ENABLE_STRING);
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32CurrentValue = enable ? 1 : 0;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("FXAA启用设置失败");
    } else {
        emit fxaaEnableChanged();
    }
}


int GraphicsConfigManager::getAAGammaCorrection() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_GAMMACORRECTION_STRING);
    return setting.u32CurrentValue == 1 ? 1 : 0;
}

void GraphicsConfigManager::setAAGammaCorrection(int enable) {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_GAMMACORRECTION_STRING);
    setting.u32CurrentValue = enable ? 1 : 0;
    
    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred("抗锯齿伽马校正设置失败");
    }
}

int GraphicsConfigManager::getAATransparency() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_REPLAY_STRING);
    return setting.u32CurrentValue & 0xFFFF;
}

void GraphicsConfigManager::setAATransparency(int mode) {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_REPLAY_STRING);
    
    if (mode < 0 || mode >= AA_MODE_REPLAY_NUM_VALUES) {
        emit errorOccurred(tr("无效的透明抗锯齿模式，支持范围: 0-%1").arg(AA_MODE_REPLAY_NUM_VALUES - 1));
        return;
    }

    switch(mode) {
        case 0: setting.u32CurrentValue = AA_MODE_REPLAY_MODE_MASK; break;
        case 1: setting.u32CurrentValue = AA_MODE_REPLAY_MODE_ALPHA_TEST; break;
        case 2: setting.u32CurrentValue = AA_MODE_REPLAY_MODE_PIXEL_KILL; break;
        default: return;
    }

    if (!applyNvidiaSetting(setting)) {
        emit errorOccurred(tr("透明抗锯齿设置失败，请检查驱动支持"));
    }
}

