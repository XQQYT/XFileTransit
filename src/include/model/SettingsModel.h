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
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(bool autoClearCache READ autoClearCache WRITE setAutoClearCache NOTIFY autoClearCacheChanged)
    Q_PROPERTY(QString cacheSize READ cacheSize WRITE setCacheSize NOTIFY cacheSizeChanged)
    Q_PROPERTY(bool autoDownload READ autoDownload WRITE setAutoDownload NOTIFY autoDownloadChanged)
    Q_PROPERTY(int autoDownloadThreshold READ autoDownloadThreshold WRITE setAutoDownloadThreshold NOTIFY autoDownloadThresholdChanged)
    Q_PROPERTY(int concurrentTransfers READ concurrentTransfers WRITE setConcurrentTransfers NOTIFY concurrentTransfersChanged)
    Q_PROPERTY(bool expandOnAction READ expandOnAction WRITE setExpandOnAction NOTIFY expandOnActionChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString changeLog READ changeLog WRITE setChangeLog NOTIFY changeLogChanged)
    Q_PROPERTY(QString releaseDate READ releaseDate WRITE setReleaseDate NOTIFY releaseDateChanged)
    Q_PROPERTY(QString newVersion READ newVersion WRITE setNewVersion NOTIFY newVersionChanged)
    Q_PROPERTY(bool isUpdateAvailable READ isUpdateAvailable WRITE setIsUpdateAvailable NOTIFY isUpdateAvailableChanged)
    Q_PROPERTY(QString updateSource READ updateSource WRITE setUpdateSource NOTIFY updateSourceChanged)
    Q_PROPERTY(bool autoCheckUpdate READ autoCheckUpdate WRITE setAutoCheckUpdate NOTIFY autoCheckUpdateChanged)
    Q_PROPERTY(bool proxyEnabled READ proxyEnabled WRITE setProxyEnabled NOTIFY proxyEnabledChanged)
    Q_PROPERTY(QString proxyAddress READ proxyAddress WRITE setProxyAddress NOTIFY proxyAddressChanged)
    Q_PROPERTY(QString proxyPort READ proxyPort WRITE setProxyPort NOTIFY proxyPortChanged)
    Q_PROPERTY(bool proxyAuthEnabled READ proxyAuthEnabled WRITE setProxyAuthEnabled NOTIFY proxyAuthEnabledChanged)
    Q_PROPERTY(QString proxyUsername READ proxyUsername WRITE setProxyUsername NOTIFY proxyUsernameChanged)
    Q_PROPERTY(QString proxyPassword READ proxyPassword WRITE setProxyPassword NOTIFY proxyPasswordChanged)
    Q_PROPERTY(QString proxyTestResult READ proxyTestResult WRITE setProxyTestResult NOTIFY proxyTestResultChanged)

public:
    explicit SettingsModel(QObject *parent = nullptr);
    int currentTheme() const { return current_theme; }
    int currentLanguage() const { return current_language; }
    bool autoStart() const { return auto_start; }
    QString cachePath() const { return cache_path; }
    QString cacheSize() const { return cache_size; }
    bool autoClearCache() const { return auto_clear_cache; }
    bool autoDownload() const { return auto_download; }
    int autoDownloadThreshold() const { return auto_download_threshold; }
    int concurrentTransfers() const { return concurrent_transfers; }
    bool expandOnAction() const { return expand_on_action; }
    QString appVersion() const { return app_version; }
    QString releaseDate() const { return release_date; }
    QString changeLog() const { return changelog; }
    QString newVersion() const { return new_version; }
    bool isUpdateAvailable() const { return is_update_available; }
    QString updateSource() const { return update_source; }
    bool autoCheckUpdate() const { return auto_check_update; }
    bool proxyEnabled() const { return proxy_enabled; }
    QString proxyAddress() const { return proxy_address; }
    QString proxyPort() const { return proxy_port; }
    bool proxyAuthEnabled() const { return proxy_auth_enabled; }
    QString proxyUsername() const { return proxy_username; }
    QString proxyPassword() const { return proxy_password; }
    QString proxyTestResult() const { return proxy_test_result; }

    void setCurrentTheme(int theme);
    void setCurrentLanguage(int language);
    void setAutoStart(const bool enable);
    void setCachePath(const QUrl &url);
    void setCacheSize(const QString &size);
    void setAutoClearCache(bool enable);
    void setAutoDownload(bool enable);
    void setAutoDownloadThreshold(int threshold);
    void setConcurrentTransfers(int transfers);
    void setExpandOnAction(bool expand);
    void setChangeLog(const QString &log);
    void setNewVersion(const QString &new_version);
    void setReleaseDate(const QString &time);
    void setIsUpdateAvailable(bool available);
    void setUpdateSource(const QString &us);
    void setAutoCheckUpdate(bool enable);
    void setProxyEnabled(bool enable);
    void setProxyAddress(const QString &address);
    void setProxyPort(const QString &port);
    void setProxyAuthEnabled(bool enable);
    void setProxyUsername(const QString &username);
    void setProxyPassword(const QString &password);
    void setProxyTestResult(QString result);
    void setQmlEngine(QQmlEngine *engine);

    Q_INVOKABLE void initSettings();
    Q_INVOKABLE void clearCache();
    Q_INVOKABLE void checkUpdate(bool show_in_new);
    Q_INVOKABLE void updateSoftware();
    Q_INVOKABLE void restartApplication();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void testProxyConnection();
    Q_INVOKABLE void cancelTestProxy();

signals:
    void currentThemeChanged(int theme);
    void currentLanguageChanged(int language);
    void autoStartChanged(bool enable);
    void cachePathChanged(const QString &path);
    void cacheSizeChanged(const QString &size);
    void autoClearCacheChanged(bool enable);
    void autoDownloadChanged(bool enable);
    void autoDownloadThresholdChanged(int threshold);
    void concurrentTransfersChanged(int transfers);
    void expandOnActionChanged(bool expand);
    void changeLogChanged(const QString &log);
    void newVersionChanged(const QString &new_version);
    void releaseDateChanged(const QString &time);
    void isUpdateAvailableChanged(bool available);
    void updateSourceChanged(const QString &update_source);
    void autoCheckUpdateChanged(const bool enabel);
    void proxyEnabledChanged(bool enable);
    void proxyAddressChanged(const QString &address);
    void proxyPortChanged(const QString &port);
    void proxyAuthEnabledChanged(bool enable);
    void proxyUsernameChanged(const QString &username);
    void proxyPasswordChanged(const QString &password);
    void proxyTestResultChanged(QString result);
    void cacheInfoDone(QString used, QString free_size, QString total);
    void cacheMoveDone();
    void settingsChanged(Settings::Item item, QVariant value);
    void downloadProgress(float progress);
    void downloadDone();
    void Error(QString error_msg);
    void versionInfoShow(QString msg);
    void updateOutput(QString output);
    void testProxyDone();

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
    void beginLoadConfig(Settings::SettingsGroup group);
    void endLoadConfig(Settings::SettingsGroup group);
    void setAutoStartImpl(const bool enable);
    bool checkAutoStart();

private:
    // general
    int current_theme;    // 0: light, 1: dark
    int current_language; // 0: Chinese, 1: English
    bool auto_start;
    // file
    QUrl cache_url;
    QString cache_path;
    QString cache_size;
    bool auto_clear_cache;
    // transfer
    bool auto_download;
    int auto_download_threshold;
    int concurrent_transfers;
    // notifity
    bool expand_on_action;
    // about
    const QString app_version = AppVersion::string;
    QString new_version;
    QString release_date;
    QString changelog;
    bool is_update_available;
    QString update_source;
    bool auto_check_update;
    bool proxy_enabled;
    QString proxy_address;
    QString proxy_port;
    bool proxy_auth_enabled;
    QString proxy_username;
    QString proxy_password;
    QString proxy_test_result;

    QQmlEngine *qml_engine = nullptr;
    QTranslator *translator;

    QTimer *cache_size_updater;
    QTimer *flush_config_timer;

    UpdateManager update_manager;

    VersionInfo new_version_info;

    bool grout_init_flags[Settings::group_count]{false};
};

#endif
