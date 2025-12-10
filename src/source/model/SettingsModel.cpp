#include "model/SettingsModel.h"
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>
#include <algorithm>

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

void SettingsModel::setCachePath(const QString &path)
{
    if (cache_path != path)
    {
        cache_path = path;
        emit cachePathChanged(path);
    }
}

void SettingsModel::setCacheSize(quint64 size)
{
    if (cache_size != size)
    {
        cache_size = size;
        emit cacheSizeChanged(size);
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
