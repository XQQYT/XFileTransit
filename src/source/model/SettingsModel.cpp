#include "model/SettingsModel.h"
#include "driver/impl/FileUtility.h"
#include "control/EventBusManager.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>
#include <algorithm>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtCore/QProcess>
#include <QtCore/QSettings>

SettingsModel::SettingsModel(QObject *parent)
    : QObject(parent), translator(new QTranslator(this))
{
    changelog = AppVersion::zh_change_log;
    EventBusManager::instance().subscribe("/settings/item_config_reslut", std::bind(
                                                                              &SettingsModel::onConfigResult,
                                                                              this,
                                                                              std::placeholders::_1,
                                                                              std::placeholders::_2));
    cache_size_updater = new QTimer(this);
    cache_size_updater->setInterval(5000);
    connect(cache_size_updater, &QTimer::timeout, [=]()
            {
            if (!cache_path.isEmpty())
            {
                uint64_t cache_size = FileSystemUtils::calculateFolderSize(cache_path.toStdString());
                setCacheSize(QString::fromStdString(FileSystemUtils::formatFileSize(cache_size)));
            } });
    cache_size_updater->start();

    flush_config_timer = new QTimer(this);
    flush_config_timer->setInterval(1000);
    flush_config_timer->setSingleShot(true);
    connect(flush_config_timer, &QTimer::timeout, [=]()
            { EventBusManager::instance().publish("/settings/write_into_file"); });
}

void SettingsModel::initSettings()
{
    std::vector<uint8_t> groups = {
        static_cast<uint8_t>(Settings::SettingsGroup::General),
        static_cast<uint8_t>(Settings::SettingsGroup::File),
        static_cast<uint8_t>(Settings::SettingsGroup::Transfer),
        static_cast<uint8_t>(Settings::SettingsGroup::Notification),
        static_cast<uint8_t>(Settings::SettingsGroup::About),
    };
    EventBusManager::instance().publish("/settings/get_item_config", groups);
}

void SettingsModel::setQmlEngine(QQmlEngine *engine)
{
    qml_engine = engine;
}

void SettingsModel::setCurrentTheme(int theme)
{
    if (current_theme != theme)
    {
        current_theme = theme;
        emit currentThemeChanged(theme);                    // qml
        emit settingsChanged(Settings::Item::Theme, theme); // model
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::General)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::General), std::string("theme"), std::to_string(theme));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setCurrentLanguage(int language)
{
    if (current_language != language)
    {
        current_language = language;
        QApplication::removeTranslator(translator);

        if (language == 1)
        {
            if (translator->load(":/translations/en.qm"))
            {
                QApplication::installTranslator(translator);
            }
            changelog = AppVersion::en_change_log;
        }
        else
        {
            if (translator->load(":/translations/zh_CN.qm"))
            {
                QApplication::installTranslator(translator);
            }
            changelog = AppVersion::zh_change_log;
        }

        if (qml_engine)
        {
            qml_engine->retranslate();
        }

        emit currentLanguageChanged(language);
        emit settingsChanged(Settings::Item::Language, language);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::General)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::General), std::string("language"), std::to_string(language));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setAutoStart(bool enable)
{
    if (enable != auto_start)
    {
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::General)])
        {
            setAutoStartImpl(enable);
        }
        auto_start = enable;
        emit autoStartChanged(auto_start);

        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::General)])
        {
            bool exec_result = checkAutoStart();
            if (auto_start != exec_result)
            {
                auto_start = exec_result;
                emit autoStartChanged(auto_start);
            }
        }
    }
}

bool SettingsModel::checkAutoStart()
{
    QString app_name = QCoreApplication::applicationName();
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    QString current_path = QCoreApplication::applicationFilePath();

    QString windows_path = QDir::toNativeSeparators(current_path);

    QString registered_path = settings.value(app_name).toString();

    QString normalized_registered_path = QDir::fromNativeSeparators(registered_path);
    QString normalized_current_path = QDir::fromNativeSeparators(windows_path);

    if (normalized_registered_path.startsWith('"') && normalized_registered_path.endsWith('"'))
    {
        normalized_registered_path = normalized_registered_path.mid(1, normalized_registered_path.length() - 2);
    }

    return (normalized_registered_path.compare(normalized_current_path, Qt::CaseInsensitive) == 0);
#else
    QString auto_start_path = QDir::homePath() + "/.config/autostart/" + app_name + ".desktop";
    return QFile::exists(auto_start_path);
#endif
}

void SettingsModel::setAutoStartImpl(const bool enable)
{
    QString app_name = QCoreApplication::applicationName();
#ifdef _WIN32
    QString app_path = QCoreApplication::applicationFilePath();

    QString windows_path = QDir::toNativeSeparators(app_path);
    if (!windows_path.startsWith('"') && !windows_path.endsWith('"'))
    {
        windows_path = "\"" + windows_path + "\"";
    }

    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if (enable)
    {
        settings.setValue(app_name, windows_path);
    }
    else
    {
        settings.remove(app_name);
    }

    settings.sync();
#else
    QString auto_start_dir = QDir::homePath() + "/.config/autostart";
    QString auto_start_path = auto_start_dir + "/" + app_name + ".desktop";

    if (enable)
    {
        QDir dir(auto_start_dir);
        if (!dir.exists())
        {
            dir.mkpath(".");
        }

        QFile desktop_file(auto_start_path);
        if (desktop_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&desktop_file);
            out << "[Desktop Entry]\n";
            out << "Type=Application\n";
            out << "Name=" << app_name << "\n";
            out << "Exec=" << QCoreApplication::applicationFilePath() << "\n";
            out << "Icon=icon.png" << "\n";
            out << "Comment=Auto start " << app_name << "\n";
            out << "X-GNOME-Autostart-enabled=true\n";
            out << "Hidden=false\n";
            desktop_file.close();
        }
    }
    else
    {
        if (QFile::exists(auto_start_path))
        {
            QFile::remove(auto_start_path);
        }
    }
#endif
}

void SettingsModel::updateCacheDiskInfo()
{
    // 获取当前所在分区的信息
    QThread::create([this]()
                    {
            auto [total, free_size] = FileSystemUtils::getDiskSpaceForFolder(cache_path.toStdString());
            uint64_t cache_size = FileSystemUtils::calculateFolderSize(cache_path.toStdString());
            emit cacheInfoDone(QString::fromStdString(FileSystemUtils::formatFileSize(total - free_size)),
                QString::fromStdString(FileSystemUtils::formatFileSize(free_size)),
                QString::fromStdString(FileSystemUtils::formatFileSize(total))); })
        ->start();
}

void SettingsModel::moveCacheDir(const std::string &des)
{
    // 迁移文件
    QThread::create([this, des]()
                    {
            FileSystemUtils::copyDirectory(des, cache_path.toStdString());
            FileSystemUtils::removeFileOrDirectory(des, false);
            emit cacheMoveDone();
            emit settingsChanged(Settings::Item::CachePath, cache_path); })
        ->start();
}

void SettingsModel::setCachePath(const QUrl &url)
{
    if (cache_url != url)
    {
        cache_url = QUrl::fromLocalFile(url.toLocalFile() + "/XFileTransitTmp");
        cache_path = cache_url.toLocalFile() + "/";

        // 先设置新临时目录，使迁移中也可以传输文件
        auto old_tmp_path = GlobalStatusManager::absolute_tmp_dir;
        GlobalStatusManager::absolute_tmp_dir = cache_path.toStdString();
        FileSystemUtils::createDirectoryRecursive(GlobalStatusManager::absolute_tmp_dir);
        emit cachePathChanged(cache_path);

        updateCacheDiskInfo();
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::File)])
        {
            moveCacheDir(old_tmp_path);
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::File), std::string("default_save_url"), cache_url.toString().toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setCacheSize(const QString &size)
{
    if (size != cache_size)
    {
        cache_size = size;
        emit cacheSizeChanged(size);
        flush_config_timer->start();
    }
}

void SettingsModel::setAutoClearCache(bool enable)
{
    if (auto_clear_cache != enable)
    {
        auto_clear_cache = enable;
        emit autoClearCacheChanged(enable);
        emit settingsChanged(Settings::Item::AutoClearCache, enable);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::File)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::File), std::string("auto_clear_cache"), std::to_string(enable));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setAutoDownload(bool enable)
{
    if (auto_download != enable)
    {
        auto_download = enable;
        emit autoDownloadChanged(enable);
        emit settingsChanged(Settings::Item::AutoDownload, enable);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::Transfer)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::Transfer), std::string("auto_download"), std::to_string(enable));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setAutoDownloadThreshold(int threshold)
{
    if (auto_download_threshold != threshold)
    {
        auto_download_threshold = threshold;
        emit autoDownloadThresholdChanged(auto_download_threshold);
        emit settingsChanged(Settings::Item::AutoDownloadThreshold, auto_download_threshold);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::Transfer)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::Transfer), std::string("auto_download_threshold"), std::to_string(threshold));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setConcurrentTransfers(int transfers)
{
    if (concurrent_transfers != transfers)
    {
        concurrent_transfers = transfers;
        emit concurrentTransfersChanged(transfers);
        emit settingsChanged(Settings::Item::ConcurrentTransfers, transfers);
        if (GlobalStatusManager::getInstance().getConnectStatus())
        {
            EventBusManager::instance().publish("/settings/send_concurrent_changed", static_cast<uint8_t>(transfers));
        }
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::Transfer)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::Transfer), std::string("concurrent_task"), std::to_string(transfers));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setExpandOnAction(bool expand)
{
    if (expand_on_action != expand)
    {
        expand_on_action = expand;
        emit expandOnActionChanged(expand);
        emit settingsChanged(Settings::Item::ExpandOnAction, expand);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::Notification)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::Notification), std::string("auto_expand"), std::to_string(expand));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setIsUpdateAvailable(bool available)
{
    if (is_update_available != available)
    {
        is_update_available = available;
        emit isUpdateAvailableChanged(available);
        emit settingsChanged(Settings::Item::IsUpdateAvailable, available);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("update_is_avaible"), std::to_string(available));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setChangeLog(const QString &log)
{
    if (changelog != log)
    {
        changelog = log;
        emit changeLogChanged(changelog);
    }
}

void SettingsModel::setNewVersion(const QString &nv)
{
    if (new_version != nv)
    {
        new_version = nv;
        emit newVersionChanged(new_version);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("new_version"), new_version.toStdString());

            flush_config_timer->start();
        }
    }
}

void SettingsModel::setReleaseDate(const QString &time)
{
    if (release_date != time)
    {
        release_date = time;
        emit releaseDateChanged(release_date);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("release_date"), release_date.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setUpdateSource(const QString &us)
{
    if (update_source != us)
    {
        update_source = us;
        emit updateSourceChanged(update_source);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("update_source"), update_source.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setAutoCheckUpdate(bool enable)
{
    if (auto_check_update != enable)
    {
        auto_check_update = enable;
        emit autoCheckUpdateChanged(enable);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("auto_check_update"), std::to_string(auto_check_update));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyEnabled(bool enable)
{
    if (proxy_enabled != enable)
    {
        proxy_enabled = enable;
        emit proxyEnabledChanged(proxy_enabled);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_enabled"), std::to_string(proxy_enabled));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyAddress(const QString &address)
{
    if (proxy_address != address)
    {
        proxy_address = address;
        emit proxyAddressChanged(proxy_address);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_address"), proxy_address.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyPort(const QString &port)
{
    if (proxy_port != port)
    {
        proxy_port = port;
        emit proxyPortChanged(proxy_address);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_port"), proxy_port.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyAuthEnabled(bool enable)
{
    if (proxy_auth_enabled != enable)
    {
        proxy_auth_enabled = enable;
        emit proxyAuthEnabledChanged(proxy_auth_enabled);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_auth_enabled"), std::to_string(proxy_auth_enabled));
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyUsername(const QString &username)
{
    if (proxy_username != username)
    {
        proxy_username = username;
        emit proxyAddressChanged(proxy_username);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_username"), proxy_username.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyPassword(const QString &password)
{
    if (proxy_password != password)
    {
        proxy_password = password;
        emit proxyAddressChanged(proxy_password);
        if (grout_init_flags[Settings::to_uint8(Settings::SettingsGroup::About)])
        {
            QByteArray base64_array = proxy_password.toUtf8().toBase64();
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("proxy_password"), base64_array.toStdString());
            flush_config_timer->start();
        }
    }
}

void SettingsModel::setProxyTestResult(QString result)
{
    if (proxy_test_result != result)
    {
        proxy_test_result = result;
        emit proxyTestResultChanged(proxy_test_result);
    }
}

void SettingsModel::beginLoadConfig(Settings::SettingsGroup group)
{
    grout_init_flags[Settings::to_uint8(group)] = false;
}

void SettingsModel::endLoadConfig(Settings::SettingsGroup group)
{
    grout_init_flags[Settings::to_uint8(group)] = true;
}

void SettingsModel::onConfigResult(uint8_t group, std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{

    QMetaObject::invokeMethod(this, [=]()
                              {
            Settings::SettingsGroup g = static_cast<Settings::SettingsGroup>(group);
            switch (g)
            {
            case Settings::SettingsGroup::General:
                setGeneralConfig(config);
                break;
            case Settings::SettingsGroup::File:
                setFileConfig(config);
                break;
            case Settings::SettingsGroup::Transfer:
                setTransitConfig(config);
                break;
            case Settings::SettingsGroup::Notification:
                setNotificationConfig(config);
                break;
            case Settings::SettingsGroup::About:
                setAboutConfig(config);
                break;
            default:
                break;
            } });
}

void SettingsModel::setGeneralConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    beginLoadConfig(Settings::SettingsGroup::General);

    setAutoStart(checkAutoStart());
    setCurrentLanguage(std::stoi((*config)["language"]));
    setCurrentTheme(std::stoi((*config)["theme"]));

    endLoadConfig(Settings::SettingsGroup::General);
}

void SettingsModel::setFileConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    beginLoadConfig(Settings::SettingsGroup::File);

    QString default_save_url = QString::fromStdString((*config)["default_save_url"]);
    if (default_save_url.isEmpty())
    {
        QString tmp_dir = QString::fromStdString(GlobalStatusManager::absolute_tmp_dir);
        cache_url = QUrl::fromLocalFile(tmp_dir);
        cache_path = tmp_dir;
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::File), std::string("default_save_url"), cache_url.toString().toStdString());
        flush_config_timer->start();
    }
    else
    {
        cache_url = default_save_url;
        cache_path = cache_url.toLocalFile();
    }
    GlobalStatusManager::absolute_tmp_dir = cache_path.toStdString();
    emit cachePathChanged(cache_path);
    updateCacheDiskInfo();

    setAutoClearCache(std::stoi((*config)["auto_clear_cache"]));

    endLoadConfig(Settings::SettingsGroup::File);
}

void SettingsModel::setTransitConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    beginLoadConfig(Settings::SettingsGroup::Transfer);

    setAutoDownload(std::stoi((*config)["auto_download"]));
    setAutoDownloadThreshold(std::stoi((*config)["auto_download_threshold"]));
    setConcurrentTransfers(std::stoi((*config)["concurrent_task"]));

    endLoadConfig(Settings::SettingsGroup::Transfer);
}

void SettingsModel::setNotificationConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    beginLoadConfig(Settings::SettingsGroup::Notification);

    setExpandOnAction(std::stoi((*config)["auto_expand"]));

    beginLoadConfig(Settings::SettingsGroup::Notification);
}

void SettingsModel::setAboutConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    beginLoadConfig(Settings::SettingsGroup::About);

    setUpdateSource(QString::fromStdString((*config)["update_source"]));
    setAutoCheckUpdate(std::stoi((*config)["auto_check_update"]));
    setProxyEnabled(std::stoi((*config)["proxy_enabled"]));
    setProxyAddress(QString::fromStdString((*config)["proxy_address"]));
    setProxyPort(QString::fromStdString((*config)["proxy_port"]));
    setProxyAuthEnabled(std::stoi((*config)["proxy_auth_enabled"]));
    setProxyUsername(QString::fromStdString((*config)["proxy_username"]));
    auto password_array = QByteArray::fromBase64(QByteArray::fromStdString((*config)["proxy_password"]));
    setProxyPassword(password_array);
    bool update_avaible = std::stoi((*config)["update_is_avaible"]);
    if (update_avaible)
    {
        setNewVersion(QString::fromStdString((*config)["new_version"]));
        setReleaseDate(QString::fromStdString((*config)["release_date"]));
    }
    // 不采用配置文件中的值，为了用户每次都需要检查更新，防止新版本过时
    is_update_available = false;
    emit isUpdateAvailableChanged(false);
    if (auto_check_update)
    {
        int64_t last_check_update = std::stoll((*config)["last_check_update"]);
        qint64 timestamp_s = QDateTime::currentDateTime().toSecsSinceEpoch();

        qint64 diff_seconds = timestamp_s - last_check_update;

        if (diff_seconds > 86400)
        {
            (*config)["last_check_update"] = std::to_string(timestamp_s);
            EventBusManager::instance().publish("/settings/update_settings_value",
                                                static_cast<uint8_t>(Settings::SettingsGroup::About), std::string("last_check_update"), std::to_string(timestamp_s));
            checkUpdate(true);
        }
    }

    endLoadConfig(Settings::SettingsGroup::About);
}
void SettingsModel::clearCache()
{
    QDir dir(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir));

    if (!dir.exists())
    {
        return;
    }

    dir.removeRecursively();
    QDir().mkpath(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir));
}

int compareVersions(const QString &versionA, const QString &versionB)
{
    QVersionNumber v1 = QVersionNumber::fromString(versionA.mid(1));
    QVersionNumber v2 = QVersionNumber::fromString(versionB.mid(1));

    if (v1 > v2)
        return 1;
    if (v1 < v2)
        return -1;
    return 0;
}

void SettingsModel::checkUpdate(bool show_in_new)
{
    if (proxy_enabled)
    {
        update_manager.setProxy(proxy_address, proxy_port, proxy_username, proxy_password);
    }
    else
    {
        update_manager.removeProxy();
    }
    connect(&update_manager, &UpdateManager::versionJsonParsedDone, [=](VersionInfo version_info)
            { if (compareVersions(version_info.lastest_version, AppVersion::string) > 0)
    {
        new_version_info = version_info;
        setIsUpdateAvailable(true);
        setChangeLog(current_language ? version_info.en_changelog : version_info.zh_changelog);
        setNewVersion(version_info.lastest_version);
        setReleaseDate(version_info.release_date);
        emit versionInfoShow(tr("发现新版本"));
    }
        else
    {
        if(!show_in_new)
        {
            emit versionInfoShow(tr("当前已是最新版本"));
        }
    } });

    connect(&update_manager, &UpdateManager::downloadError, [=](const QString &error_msg)
            { emit Error(error_msg); });

    if (update_source == "github")
    {
        update_manager.downloadVersionJson(GitPlatform::Github, "XQQYT", "XFileTransit", "master", "src/res/version/version.json");
    }
    else if (update_source == "gitcode")
    {
        update_manager.downloadVersionJson(GitPlatform::GitCode, "XQQYT", "XFileTransit", "master", "src/res/version/version.json");
    }
    else
    {
        emit versionInfoShow(tr("更新源错误"));
    }
}

void SettingsModel::onDownloadProgress(quint64 received, quint64 total)
{
    double percent;
    if (total > 0)
    {
        if (received == total)
        {
            emit downloadDone();
            return;
        }
        percent = (static_cast<double>(received) / total);
    }
    else
    {
        percent = 0.0f;
    }
    emit downloadProgress(percent);
}

void SettingsModel::onPackageDownloadDone(QString path)
{
#ifdef _WIN32
    if (!QFile::exists(path))
    {
        LOG_ERROR("文件不存在:" << path.toStdString());
        return;
    }
    QProcess::startDetached(path);
    QCoreApplication::quit();
#else
    QFile scriptFile(":/tools/updateLinux.sh");
    QString tempScriptPath = QDir::tempPath() + "/updateXFileTransit.sh";
    QFile temp_script(tempScriptPath);

    QFile::remove(tempScriptPath);
    scriptFile.copy(tempScriptPath);
    temp_script.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

    QString updatePackage = path;
    QString installDir = QCoreApplication::applicationDirPath();
    QDir dir(installDir);

    if (dir.cdUp())
    {
        installDir = dir.absolutePath();
    }

    QProcess *process = new QProcess();

    connect(process, &QProcess::readyReadStandardOutput, [=]()
            {
            QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
            emit updateOutput(output); });

    connect(process, &QProcess::readyReadStandardError, [=]()
            {
            QString error = QString::fromLocal8Bit(process->readAllStandardError());
            if (!error.trimmed().isEmpty()) {
                emit updateOutput(tr("错误: %1").arg(error));
            } });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                if (exitStatus == QProcess::NormalExit && exitCode == 0)
                {
                    emit updateOutput(tr("更新成功，请重启应用"));
                }
                else
                {
                    emit updateOutput(tr("更新失败"));
                }

                process->deleteLater();
            });
    process->setProgram("/bin/bash");
    process->setArguments({tempScriptPath, updatePackage, installDir});
    process->start();

    if (!process->waitForStarted(5000))
    {
        emit updateOutput(tr("更新脚本启动失败"));
    }
#endif
}

void SettingsModel::updateSoftware()
{
    if (proxy_enabled)
    {
        update_manager.setProxy(proxy_address, proxy_port, proxy_username, proxy_password);
    }
    else
    {
        update_manager.removeProxy();
    }
    connect(&update_manager, &UpdateManager::downloadPackageDone, this, &SettingsModel::onPackageDownloadDone);

    connect(&update_manager, &UpdateManager::downloadProgress, this, &SettingsModel::onDownloadProgress);

    connect(&update_manager, &UpdateManager::downloadError, [=](const QString &error_msg)
            { emit Error(error_msg); });

    if (update_source == "github")
    {
#ifdef _WIN32
        update_manager.downloadPackage(new_version_info.win_github_url);
#else
        update_manager.downloadPackage(new_version_info.linux_github_url);
#endif
    }
    else if (update_source == "gitcode")
    {
#ifdef _WIN32
        update_manager.downloadPackage(new_version_info.win_gitcode_url);
#else
        update_manager.downloadPackage(new_version_info.linux_gitcode_url);
#endif
    }
    else
    {
        emit versionInfoShow(tr("更新源错误"));
    }
}

void SettingsModel::restartApplication()
{
    QCoreApplication::quit();
}

void SettingsModel::cancelDownload()
{
    update_manager.cancelDownload();
}

void SettingsModel::testProxyConnection()
{
    connect(&update_manager, &UpdateManager::testResult, this, [=](bool ret)
            { setProxyTestResult(ret ? tr("连接成功"):tr("连接失败"));
            emit testProxyDone(); }, Qt::SingleShotConnection);
    connect(&update_manager, &UpdateManager::testError, this, [=](const QString error)
            { emit Error(error); }, Qt::SingleShotConnection);
    update_manager.testProxy(proxy_address, proxy_port, proxy_username, proxy_password);
}

void SettingsModel::cancelTestProxy()
{
    update_manager.cancelTestProxy();
}