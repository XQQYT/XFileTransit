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

SettingsModel::SettingsModel(QObject *parent)
    : QObject(parent), translator(new QTranslator(this))
{
    app_version = AppVersion::string;
    EventBusManager::instance().subscribe("/settings/item_config_reslut", std::bind(
                                                                              &SettingsModel::onConfigResult,
                                                                              this,
                                                                              std::placeholders::_1,
                                                                              std::placeholders::_2));
    cache_size_updater = new QTimer(this);
    cache_size_updater->setInterval(5000);
    connect(cache_size_updater, &QTimer::timeout, [=]()
            {
        if(!cache_path.isEmpty())
        {
        uint64_t cache_size = FileSystemUtils::calculateFolderSize(cache_path.toStdString());
        setCacheSize(QString::fromStdString(FileSystemUtils::formatFileSize(cache_size)));
        } });
    cache_size_updater->start();
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
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::General), std::string("theme"), std::to_string(theme));
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
        }
        else
        {
            if (translator->load(":/translations/zh_CN.qm"))
            {
                QApplication::installTranslator(translator);
            }
        }

        if (qml_engine)
        {
            qml_engine->retranslate();
        }

        emit currentLanguageChanged(language);
        emit settingsChanged(Settings::Item::Language, language);
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::General), std::string("language"), std::to_string(language));
    }
}

void SettingsModel::updateCacheDiskInfo()
{
    // 获取当前所在分区的信息
    QThread::create([this]()
                    {
                        auto [total, free_size] = FileSystemUtils::getDiskSpaceForFolder(cache_path.toStdString()); 
                        uint64_t cache_size = FileSystemUtils::calculateFolderSize(cache_path.toStdString());
                        emit cacheInfoDone(QString::fromStdString(FileSystemUtils::formatFileSize(total-free_size)),
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

        cache_url = url;
        cache_path = url.toLocalFile() + "/";

        // 先设置新临时目录，使迁移中也可以传输文件
        auto old_tmp_path = GlobalStatusManager::absolute_tmp_dir;
        GlobalStatusManager::absolute_tmp_dir = cache_path.toStdString();
        FileSystemUtils::createDirectoryRecursive(GlobalStatusManager::absolute_tmp_dir);
        emit cachePathChanged(cache_path);

        updateCacheDiskInfo();
        moveCacheDir(old_tmp_path);
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::File), std::string("default_save_url"), cache_url.toString().toStdString());
    }
}

void SettingsModel::setCacheSize(const QString &size)
{
    if (size != cache_size)
    {
        cache_size = size;
        emit cacheSizeChanged(size);
    }
}

void SettingsModel::setAutoClearCache(bool enable)
{
    if (auto_clear_cache != enable)
    {
        auto_clear_cache = enable;
        emit autoClearCacheChanged(enable);
        emit settingsChanged(Settings::Item::AutoClearCache, enable);
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::File), std::string("auto_clear_cache"), std::to_string(enable));
    }
}

void SettingsModel::setAutoDownload(bool enable)
{
    if (auto_download != enable)
    {
        auto_download = enable;
        emit autoDownloadChanged(enable);
        emit settingsChanged(Settings::Item::AutoDownload, enable);
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::Transfer), std::string("auto_download"), std::to_string(enable));
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
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::Transfer), std::string("concurrent_task"), std::to_string(transfers));
    }
}

void SettingsModel::setExpandOnAction(bool expand)
{
    if (expand_on_action != expand)
    {
        expand_on_action = expand;
        emit expandOnActionChanged(expand);
        emit settingsChanged(Settings::Item::ExpandOnAction, expand);
        EventBusManager::instance().publish("/settings/update_settings_value",
                                            static_cast<uint8_t>(Settings::SettingsGroup::Notification), std::string("auto_expand"), std::to_string(expand));
    }
}

void SettingsModel::setAppVersion(const QString &version)
{
    if (app_version != version)
    {
        app_version = version;
        emit appVersionChanged(version);
        emit settingsChanged(Settings::Item::AppVersion, version);
    }
}

void SettingsModel::setIsUpdateAvailable(bool available)
{
    if (is_update_available != available)
    {
        is_update_available = available;
        emit isUpdateAvailableChanged(available);
        emit settingsChanged(Settings::Item::IsUpdateAvailable, available);
    }
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
    setCurrentLanguage(std::stoi((*config)["language"]));
    setCurrentTheme(std::stoi((*config)["theme"]));
}

void SettingsModel::setFileConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    QString default_save_url = QString::fromStdString((*config)["default_save_url"]);
    if (default_save_url.isEmpty())
    {
        QString tmp_dir = QString::fromStdString(GlobalStatusManager::absolute_tmp_dir);
        cache_url = QUrl::fromLocalFile(tmp_dir);
        cache_path = tmp_dir;
    }
    else
    {
        cache_url = default_save_url;
        cache_path = cache_url.toLocalFile();
    }
    GlobalStatusManager::absolute_tmp_dir = cache_path.toStdString() + "/";
    emit cachePathChanged(cache_path);
    updateCacheDiskInfo();

    setAutoClearCache(std::stoi((*config)["auto_clear_cache"]));
}

void SettingsModel::setTransitConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    setAutoDownload(std::stoi((*config)["auto_download"]));
    setConcurrentTransfers(std::stoi((*config)["concurrent_task"]));
}

void SettingsModel::setNotificationConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    setExpandOnAction(std::stoi((*config)["auto_expand"]));
}

void SettingsModel::setAboutConfig(std::shared_ptr<std::unordered_map<std::string, std::string>> config)
{
    setIsUpdateAvailable(std::stoi((*config)["update_is_avaible"]));
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
