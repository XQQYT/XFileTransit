#ifndef _SETTINGSMODEL_H
#define _SETTINGSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTranslator>
#include <QtQml/QQmlApplicationEngine>
#include <string>
#include <cstdint>

#include "control/GlobalStatusManager.h"

class SettingsModel : public QObject
{
    Q_OBJECT

    // 定义所有属性
    Q_PROPERTY(int currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(int currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(bool autoDownload READ autoDownload WRITE setAutoDownload NOTIFY autoDownloadChanged)
    Q_PROPERTY(int concurrentTransfers READ concurrentTransfers WRITE setConcurrentTransfers NOTIFY concurrentTransfersChanged)
    Q_PROPERTY(bool enableEncryption READ enableEncryption WRITE setEnableEncryption NOTIFY enableEncryptionChanged)
    Q_PROPERTY(bool expandOnAction READ expandOnAction WRITE setExpandOnAction NOTIFY expandOnActionChanged)
    Q_PROPERTY(QString appVersion READ appVersion WRITE setAppVersion NOTIFY appVersionChanged)
    Q_PROPERTY(bool isUpdateAvailable READ isUpdateAvailable WRITE setIsUpdateAvailable NOTIFY isUpdateAvailableChanged)

public:
    explicit SettingsModel(QObject *parent = nullptr);

    int currentTheme() const { return current_theme; }
    int currentLanguage() const { return current_language; }
    QString cachePath() const { return cache_path; }
    bool autoDownload() const { return auto_download; }
    int concurrentTransfers() const { return concurrent_transfers; }
    bool enableEncryption() const { return enable_encryption; }
    bool expandOnAction() const { return expand_on_action; }
    QString appVersion() const { return app_version; }
    bool isUpdateAvailable() const { return is_update_available; }

    void setCurrentTheme(int theme);
    void setCurrentLanguage(int language);
    void setCachePath(const QUrl &url);
    void setAutoDownload(bool enable);
    void setConcurrentTransfers(int transfers);
    void setEnableEncryption(bool enable);
    void setExpandOnAction(bool expand);
    void setAppVersion(const QString &version);
    void setIsUpdateAvailable(bool available);
    void setQmlEngine(QQmlEngine *engine);

signals:
    void currentThemeChanged(int theme);
    void currentLanguageChanged(int language);
    void cachePathChanged(const QString &path);
    void autoDownloadChanged(bool enable);
    void concurrentTransfersChanged(int transfers);
    void enableEncryptionChanged(bool enable);
    void expandOnActionChanged(bool expand);
    void appVersionChanged(const QString &version);
    void isUpdateAvailableChanged(bool available);
    void cacheInfoDone(QString used, QString free_size, QString total);
    void cacheMoveDone();
    void settingsChanged(Settings::Item item, QVariant value);

private:
    int current_theme = 0;    // 0: light, 1: dark
    int current_language = 0; // 0: English, 1: Chinese
    QUrl cache_url;
    QString cache_path = "";
    bool auto_download = false;
    int concurrent_transfers = 3;
    bool enable_encryption = true;
    bool expand_on_action = true;
    QString app_version = "1.0.0";
    bool is_update_available = false;

    QQmlEngine *qml_engine = nullptr;
    QTranslator *translator;
};

#endif
