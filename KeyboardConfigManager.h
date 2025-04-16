#pragma once
#include <QObject>
#include <Windows.h>

class KeyboardConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
    Q_PROPERTY(int repeatRate READ repeatRate WRITE setRepeatRate NOTIFY repeatRateChanged)

public:
    explicit KeyboardConfigManager(QObject* parent = nullptr);

    int repeatDelay() const;
    Q_INVOKABLE void setRepeatDelay(int delay);

    int repeatRate() const;
    Q_INVOKABLE void setRepeatRate(int rate);

signals:
    void repeatDelayChanged();
    void repeatRateChanged();
    void errorOccurred(const QString& msg);
};