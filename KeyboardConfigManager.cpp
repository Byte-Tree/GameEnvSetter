#include "KeyboardConfigManager.h"
#include <QDebug>

KeyboardConfigManager::KeyboardConfigManager(QObject* parent) : QObject(parent) {}

int KeyboardConfigManager::repeatDelay() const {
    UINT delay = 0;
    if (!SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &delay, 0)) {
        const_cast<KeyboardConfigManager*>(this)->errorOccurred(tr("无法获取键盘重复延迟设置 (错误码: %1)").arg(GetLastError()));
    }
    return delay;
}

void KeyboardConfigManager::setRepeatDelay(int delay) {
    if(delay < 0 || delay > 3) {
        emit errorOccurred(tr("重复延迟需在0-3之间"));
        return;
    }
    if(SystemParametersInfo(SPI_SETKEYBOARDDELAY, static_cast<UINT>(delay), nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        emit repeatDelayChanged();
    } else {
        emit errorOccurred(tr("设置键盘延迟失败"));
    }
}

int KeyboardConfigManager::repeatRate() const {
    UINT speed = 0;
    if (!SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &speed, 0)) {
        const_cast<KeyboardConfigManager*>(this)->errorOccurred(tr("无法获取键盘重复速度设置 (错误码: %1)").arg(GetLastError()));
    }
    // 将原始数值转换为字符/秒（保持0-31范围）
    float cps = 2.5f + (speed * (30.0f - 2.5f) / 31.0f);
    UINT rate = speed;
    return rate;
}

void KeyboardConfigManager::setRepeatRate(int rate) {
    if(rate < 0 || rate > 31) {
        emit errorOccurred(tr("重复速度需在0-31之间"));
        return;
    }
    if(SystemParametersInfo(SPI_SETKEYBOARDSPEED, static_cast<UINT>(rate), nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        emit repeatRateChanged();
    } else {
        emit errorOccurred(tr("设置键盘速度失败"));
    }
}