#pragma once
#include <QObject>
#include <QtGlobal>
#include <QVariant>
#include <QtQmlIntegration>
#include <windows.h>

class SystemValueConverter : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit SystemValueConverter(QObject* parent = nullptr);

    static double normalizeMouseSpeed(int speed);
    static int denormalizeMouseSpeed(double normalized);
    static QVariant registryToDisplay(DWORD value);
    static DWORD displayToRegistry(const QVariant& value);
};
