#include "SystemValueConverter.h"

SystemValueConverter::SystemValueConverter(QObject* parent) : QObject(parent) {}

double SystemValueConverter::normalizeMouseSpeed(int speed) {
    return qBound(0.0, (speed - 1) * (100.0 / 19.0), 100.0);
}

int SystemValueConverter::denormalizeMouseSpeed(double normalized) {
    return qBound(1, static_cast<int>(normalized * 19.0 / 100.0 + 1), 20);
}

QVariant SystemValueConverter::registryToDisplay(DWORD value) {
    switch(value) {
        case 0x26: return QObject::tr("后台服务");
        case 0x18: return QObject::tr("前台优化");
        default: return static_cast<int>(value);
    }
}

DWORD SystemValueConverter::displayToRegistry(const QVariant& value) {
    bool ok;
    QString str = value.toString().trimmed();
    
    // 处理十六进制输入
    if(str.startsWith("0x")) {
        return str.toUInt(&ok, 16);
    }
    
    // 处理预设字符串值
    if(str == QObject::tr("后台服务")) return 0x26;
    if(str == QObject::tr("前台优化")) return 0x18;

    // 处理十进制数值
    return str.toUInt(&ok);
}