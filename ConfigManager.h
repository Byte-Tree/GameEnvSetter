#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);
    
    Q_INVOKABLE bool configExists() const;
    
    Q_INVOKABLE void saveConfig(const QVariantMap &config);
    Q_INVOKABLE void saveUserConfig(const QVariantMap &config, const QString &path);
    Q_INVOKABLE QVariantMap loadFromPath(const QString &filePath) const;
    Q_INVOKABLE QVariantMap loadConfig() const;

private:
    void saveCommonSettings(QSettings &settings, const QVariantMap &config);
    void saveGraphicsSettings(QSettings &settings, const QVariantMap &config);
    void handleSaveResult(QSettings &settings);
    void saveSettings(const QString &path, const QVariantMap &config);
signals:
    void errorOccurred(const QString &msg) const;
    void configSaved() const;
    
private:
    QString m_configPath;
};

#endif // CONFIGMANAGER_H
