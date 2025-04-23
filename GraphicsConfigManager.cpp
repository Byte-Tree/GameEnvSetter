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

QList<QPair<QString, QPair<NvU32, NvU32>>> GraphicsConfigManager::getAllNvidiaSettings() {
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
    drsSetting.u32CurrentValue = qBound(0, fps, 0x3ff);
    if(!applyNvidiaSetting(drsSetting)) {
        //emit appIdleFPSLimitChanged();
        emit errorOccurred(tr("应用应用空闲时的FPS限制失败"));
    }
}

int GraphicsConfigManager::getAnisotropicFiltering() {
    NVDRS_SETTING setting = queryNvidiaSetting(ANISO_MODE_LEVEL_STRING);
    return (setting.u32CurrentValue & ANISO_MODE_LEVEL_MASK) - 1;//qml array组件只能从0开始计数
}

void GraphicsConfigManager::setAnisotropicFiltering(int level) {
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
}

int GraphicsConfigManager::getAAModeSelector() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_SELECTOR_STRING);
    switch(setting.u32CurrentValue) {
        case AA_MODE_SELECTOR_APP_CONTROL: return 0;
        case AA_MODE_SELECTOR_OVERRIDE: return 1;
        case AA_MODE_SELECTOR_ENHANCE: return 2;
        default: return 0;
    }
}

void GraphicsConfigManager::setAAModeSelector(int mode) {
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
}

int GraphicsConfigManager::getAAModeMethod() {
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

void GraphicsConfigManager::setAAModeMethod(int mode) {
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
}

int GraphicsConfigManager::getFXAAEnable() {
    NVDRS_SETTING setting = queryNvidiaSetting(FXAA_ENABLE_STRING);
    switch(setting.u32CurrentValue) {
        case FXAA_ENABLE_OFF: return 1;
        case FXAA_ENABLE_ON:  return 2;
        default:              return 0;
    }
}

void GraphicsConfigManager::setFXAAEnable(int index) {
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
}


int GraphicsConfigManager::getAAGammaCorrection() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_GAMMACORRECTION_STRING);
    return setting.u32CurrentValue; // 直接返回原始值
}

void GraphicsConfigManager::setAAGammaCorrection(int level) {
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
}

int GraphicsConfigManager::getAATransparency() {
    NVDRS_SETTING setting = queryNvidiaSetting(AA_MODE_REPLAY_STRING);//英伟达控制面板查到的是0 0 20 36 52
    return setting.u32CurrentValue & 0x00000070;
}

void GraphicsConfigManager::setAATransparency(int mode) {
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
}


int GraphicsConfigManager::getLowLatencyMode() {
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

void GraphicsConfigManager::setLowLatencyMode(int value) {
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
}



