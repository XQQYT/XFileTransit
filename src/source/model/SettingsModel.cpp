#include "model/SettingsModel.h"
#include "driver/impl/FileUtility.h"
#include "control/EventBusManager.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>
#include <algorithm>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>

SettingsModel::SettingsModel(QObject *parent)
    : QObject(parent), translator(new QTranslator(this))
{
    app_version = AppVersion::string;
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
            qDebug() << "retranslate";
            qml_engine->retranslate();
        }

        emit currentLanguageChanged(language);
        emit settingsChanged(Settings::Item::Language, language);
    }
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
        // 获取当前所在分区的信息
        QThread::create([this]()
                        {
                        auto [total, free_size] = FileSystemUtils::getDiskSpaceForFolder(cache_path.toStdString()); 
                        emit cacheInfoDone(QString::fromStdString(FileSystemUtils::formatFileSize(total-free_size)),
                        QString::fromStdString(FileSystemUtils::formatFileSize(free_size)),
                        QString::fromStdString(FileSystemUtils::formatFileSize(total))); })
            ->start();
        // 迁移文件
        QThread::create([this, old_tmp_path]()
                        {
                        FileSystemUtils::copyDirectory(old_tmp_path, cache_path.toStdString());
                        FileSystemUtils::removeFileOrDirectory(old_tmp_path, false);
                        emit cacheMoveDone();
                        emit settingsChanged(Settings::Item::CachePath, cache_path); })
            ->start();
    }
}

void SettingsModel::setAutoDownload(bool enable)
{
    if (auto_download != enable)
    {
        auto_download = enable;
        emit autoDownloadChanged(enable);
        emit settingsChanged(Settings::Item::AutoDownload, enable);
    }
}

void SettingsModel::setConcurrentTransfers(int transfers)
{
    if (concurrent_transfers != transfers)
    {
        concurrent_transfers = transfers;
        emit concurrentTransfersChanged(transfers);
        emit settingsChanged(Settings::Item::ConcurrentTransfers, transfers);
        EventBusManager::instance().publish("/settings/send_concurrent_changed", static_cast<uint8_t>(transfers));
    }
}

void SettingsModel::setExpandOnAction(bool expand)
{
    if (expand_on_action != expand)
    {
        expand_on_action = expand;
        emit expandOnActionChanged(expand);
        emit settingsChanged(Settings::Item::ExpandOnAction, expand);
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
