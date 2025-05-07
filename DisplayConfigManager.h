#pragma once

#include <Windows.h>
#include <QObject>
#include <QString>

class DisplayConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int refreshRate READ getRefreshRate WRITE setRefreshRate NOTIFY refreshRateChanged)

public:
    explicit DisplayConfigManager(QObject *parent = nullptr);

    // 刷新率设置
    Q_INVOKABLE int getRefreshRate();
    Q_INVOKABLE void setRefreshRate(int rate);
    Q_INVOKABLE bool isDynamicRefreshRateSupported();
    Q_INVOKABLE QVariantList getAvailableRefreshRates();
    

signals:
    void errorOccurred(const QString& msg);
    void refreshRateChanged();

private:
    // 实现细节
};
