#ifndef _SETTINGSMODEL_H
#define _SETTINGSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTranslator>
#include <QtQml/QQmlApplicationEngine>
#include <QtCore/QTimer>
#include <string>
#include <cstdint>

#include "control/GlobalStatusManager.h"
#include "model/UpdateManager.h"

class SettingsModel : public QObject
{
    Q_OBJECT

    // 定义所有属性
    Q_PROPERTY(int currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(int currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(bool autoClearCache READ autoClearCache WRITE setAutoClearCache NOTIFY autoClearCacheChanged)
    Q_PROPERTY(QString cacheSize READ cacheSize WRITE setCacheSize NOTIFY cacheSizeChanged)
    Q_PROPERTY(bool autoDownload READ autoDownload WRITE setAutoDownload NOTIFY autoDownloadChanged)
    Q_PROPERTY(int concurrentTransfers READ concurrentTransfers WRITE setConcurrentTransfers NOTIFY concurrentTransfersChanged)
    Q_PROPERTY(bool expandOnAction READ expandOnAction WRITE setExpandOnAction NOTIFY expandOnActionChanged)
    Q_PROPERTY(QString appVersion READ appVersion WRITE setAppVersion NOTIFY appVersionChanged)
    Q_PROPERTY(QString changeLog READ changeLog WRITE setChangeLog NOTIFY changeLogChanged)
    Q_PROPERTY(QString newVersion READ newVersion WRITE setNewVersion NOTIFY newVersionChanged)
    Q_PROPERTY(bool isUpdateAvailable READ isUpdateAvailable WRITE setIsUpdateAvailable NOTIFY isUpdateAvailableChanged)
    Q_PROPERTY(QString updateSource READ updateSource WRITE setUpdateSource NOTIFY updateSourceChanged)

public:
    explicit SettingsModel(QObject *parent = nullptr);
    int currentTheme() const { return current_theme; }
    int currentLanguage() const { return current_language; }
    QString cachePath() const { return cache_path; }
    QString cacheSize() const { return cache_size; }
    bool autoClearCache() const { return auto_clear_cache; }
    bool autoDownload() const { return auto_download; }
    int concurrentTransfers() const { return concurrent_transfers; }
    bool expandOnAction() const { return expand_on_action; }
    QString appVersion() const { return app_version; }
    QString changeLog() const { return changelog; }
    QString newVersion() const { return new_version; }
    bool isUpdateAvailable() const { return is_update_available; }
    QString updateSource() const { return update_source; }

    void setCurrentTheme(int theme);
    void setCurrentLanguage(int language);
    void setCachePath(const QUrl &url);
    void setCacheSize(const QString &size);
    void setAutoClearCache(bool enable);
    void setAutoDownload(bool enable);
    void setConcurrentTransfers(int transfers);
    void setExpandOnAction(bool expand);
    void setAppVersion(const QString &version);
    void setChangeLog(const QString &log);
    void setNewVersion(const QString &new_version);
    void setIsUpdateAvailable(bool available);
    void setUpdateSource(const QString &us);
    void setQmlEngine(QQmlEngine *engine);

    Q_INVOKABLE void initSettings();
    Q_INVOKABLE void clearCache();
    Q_INVOKABLE void checkUpdate();
    Q_INVOKABLE void updateSoftware();
    Q_INVOKABLE void restartApplication();

signals:
    void currentThemeChanged(int theme);
    void currentLanguageChanged(int language);
    void cachePathChanged(const QString &path);
    void cacheSizeChanged(const QString &size);
    void autoClearCacheChanged(bool enable);
    void autoDownloadChanged(bool enable);
    void concurrentTransfersChanged(int transfers);
    void expandOnActionChanged(bool expand);
    void appVersionChanged(const QString &version);
    void changeLogChanged(const QString &log);
    void newVersionChanged(const QString &new_version);
    void isUpdateAvailableChanged(bool available);
    void updateSourceChanged(const QString &update_source);
    void cacheInfoDone(QString used, QString free_size, QString total);
    void cacheMoveDone();
    void settingsChanged(Settings::Item item, QVariant value);
    void downloadProgress(QString progress);
    void downloadDone();
    void downloadError(QString error_msg);
    void versionInfoShow(QString msg);
    void updateOutput(QString output);

private:
    void onConfigResult(uint8_t group, std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void setGeneralConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void setFileConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void setTransitConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void setNotificationConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void setAboutConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config);
    void updateCacheDiskInfo();
    void moveCacheDir(const std::string &des);
    void onDownloadProgress(quint64 received, quint64 total);
    void onPackageDownloadDone(QString path);

private:
    int current_theme;    // 0: light, 1: dark
    int current_language; // 0: English, 1: Chinese
    QUrl cache_url;
    QString cache_path;
    QString cache_size;
    bool auto_clear_cache;
    bool auto_download;
    int concurrent_transfers;
    bool expand_on_action;
    QString app_version;
    QString new_version;
    QString changelog;
    bool is_update_available;
    QString update_source;

    QQmlEngine *qml_engine = nullptr;
    QTranslator *translator;

    QTimer *cache_size_updater;

    UpdateManager update_manager;

    VersionInfo new_version_info;
};

#endif
