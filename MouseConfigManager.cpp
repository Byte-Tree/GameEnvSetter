#include "MouseConfigManager.h"
#include <QDebug>

MouseConfigManager::MouseConfigManager(QObject* parent) : QObject(parent) {}

int MouseConfigManager::doubleClickSpeed() const {
    return GetDoubleClickTime();
}

void MouseConfigManager::setDoubleClickSpeed(int ms) {
    int current = GetDoubleClickTime();
    if(ms == current) return;
    
    if(ms < 100 || ms > 900) {
        emit errorOccurred(tr("双击速度需在100-900毫秒之间"));
        return;
    }
    if(!updateSystemParameter(SPI_SETDOUBLECLICKTIME, ms)) {
        // 恢复原值
        SystemParametersInfo(SPI_SETDOUBLECLICKTIME, current, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    }
}

int MouseConfigManager::mouseSpeed() const {
    int speed = 10;
    if (!SystemParametersInfo(SPI_GETMOUSESPEED, 0, &speed, 0)) {
        const_cast<MouseConfigManager*>(this)->errorOccurred(tr("无法获取鼠标指针移动速度设置"));
        return speed;
    }
    return speed;
}

void MouseConfigManager::setMouseSpeed(int speed) {
    if(speed < 1 || speed > 20) {
        emit errorOccurred(tr("鼠标速度需在1-20之间"));
        return;
    }
    if(!updateSystemParameter(SPI_SETMOUSESPEED, speed)) {
        DWORD errCode = GetLastError();
        if(errCode == ERROR_ACCESS_DENIED) {
            emit errorOccurred(tr("需要管理员权限才能修改此设置"));
        }
        return;
    }
}

bool MouseConfigManager::enhancePointerPrecision() const {
    DWORD params[3] = {0};
    if (!SystemParametersInfo(SPI_GETMOUSE, 0, params, 0)) {
        const_cast<MouseConfigManager*>(this)->errorOccurred(tr("无法获取鼠标增强指针精度设置"));
        return false;
    }
    return params[2] != 0;
}

void MouseConfigManager::setEnhancePointerPrecision(bool enabled) {
    bool current = enhancePointerPrecision();
    if(enabled == current) return;
    
    updateSystemParameter(SPI_SETMOUSE, enabled);
}

bool MouseConfigManager::updateSystemParameter(UINT uiAction, int value) const {
    DWORD params[3] = {0};
    BOOL result;
    if(uiAction == SPI_SETMOUSESPEED) {
        result = SystemParametersInfo(uiAction, 0, (LPVOID)(INT_PTR)value, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    } else if(uiAction == SPI_SETDOUBLECLICKTIME) {
        result = SystemParametersInfo(uiAction, value, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    } else {
        // 获取当前鼠标参数
        SystemParametersInfo(SPI_GETMOUSE, 0, params, 0);
        
        // 仅修改增强指针精度标志（第三个参数）
        params[2] = static_cast<DWORD>(value);
        result = SystemParametersInfo(uiAction, 0, params, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    }
    
    if(!result) {
        DWORD errCode = GetLastError();
        LPSTR errMsg = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      nullptr, errCode, 0, (LPSTR)&errMsg, 0, nullptr);
        const_cast<MouseConfigManager*>(this)->errorOccurred(
            tr("系统参数更新失败 (错误码%1): %2")
                .arg(errCode)
                .arg(QString::fromLocal8Bit(errMsg)));
        LocalFree(errMsg);
        return false;
    }
    return true;
}
