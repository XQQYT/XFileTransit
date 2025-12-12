#include "model/SettingsModel.h"
#include "driver/impl/FileUtility.h"
#include "control/GlobalStatusManager.h"
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>
#include <algorithm>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>

SettingsModel::SettingsModel(QObject *parent)
    : QObject(parent), translator(new QTranslator(this))
{
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
        emit currentThemeChanged(theme);
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
    }
}

void SettingsModel::setCachePath(const QUrl &url)
{
    if (cache_url != url)
    {
        cache_url = url;
        cache_path = url.toLocalFile();
        emit cachePathChanged(cache_path);
        QThread::create([this]()
                        {
                        auto [total, free_size] = FileSystemUtils::getDiskSpaceForFolder(cache_path.toStdString()); 
                        emit cacheInfoDone(QString::fromStdString(FileSystemUtils::formatFileSize(total-free_size)),
                         QString::fromStdString(FileSystemUtils::formatFileSize(free_size)),
                        QString::fromStdString(FileSystemUtils::formatFileSize(total))); })
            ->start();
        QThread::create([this]()
                        {
                        auto old_tmp_path = GlobalStatusManager::absolute_tmp_dir;
                        FileSystemUtils::copyDirectory(GlobalStatusManager::absolute_tmp_dir, cache_path.toStdString());
                        GlobalStatusManager::absolute_tmp_dir = cache_path.toStdString();
                        FileSystemUtils::removeFileOrDirectory(old_tmp_path, false);
                        emit cacheMoveDone(); })
            ->start();
    }
}

void SettingsModel::setAutoDownload(bool enable)
{
    if (auto_download != enable)
    {
        auto_download = enable;
        emit autoDownloadChanged(enable);
    }
}

void SettingsModel::setConcurrentTransfers(int transfers)
{
    if (concurrent_transfers != transfers)
    {
        concurrent_transfers = transfers;
        emit concurrentTransfersChanged(transfers);
    }
}

void SettingsModel::setEnableEncryption(bool enable)
{
    if (enable_encryption != enable)
    {
        enable_encryption = enable;
        emit enableEncryptionChanged(enable);
    }
}

void SettingsModel::setExpandOnAction(bool expand)
{
    if (expand_on_action != expand)
    {
        expand_on_action = expand;
        emit expandOnActionChanged(expand);
    }
}

void SettingsModel::setAppVersion(const QString &version)
{
    if (app_version != version)
    {
        app_version = version;
        emit appVersionChanged(version);
    }
}

void SettingsModel::setIsUpdateAvailable(bool available)
{
    if (is_update_available != available)
    {
        is_update_available = available;
        emit isUpdateAvailableChanged(available);
    }
}
