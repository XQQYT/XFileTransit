#include "model/UpdateManager.h"
#include "common/DebugOutputer.h"
#include "control/GlobalStatusManager.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#include <QtCore/QFile>

UpdateManager::UpdateManager(QObject *parent) : QObject(parent),
                                                git_downloader(std::make_unique<GitDownloader>(parent)),
                                                version_parser(std::make_unique<VersionParser>()) {
                                                };

QString UpdateManager::buildUrl(const GitPlatform platform, const QString &owner,
                                const QString &repo, const QString &branch,
                                const QString &file_path) const
{
    switch (platform)
    {
    case GitPlatform::Github:
        return QString("https://raw.githubusercontent.com/%1/%2/%3/%4")
            .arg(owner)
            .arg(repo)
            .arg(branch)
            .arg(file_path);
    case GitPlatform::Gitee:
        return QString("https://gitee.com/%1/%2/raw/%3/%4")
            .arg(owner)
            .arg(repo)
            .arg(branch)
            .arg(file_path);
    default:
        LOG_ERROR("Invalid GitPlatform");
        return QString();
    }
}

void UpdateManager::downloadVersionJson(const GitPlatform platform, const QString &owner,
                                        const QString &repo, const QString &branch,
                                        const QString &file_path)
{
    QString url = buildUrl(platform, owner, repo, branch, file_path);

    git_downloader->setCallbacks([=](quint64 n1, quint64 n2)
                                 { emit downloadProgress(n1, n2); },
                                 [=](const QByteArray &data)
                                 {
                                     emit versionJsonParsedDone(version_parser->parse(data));
                                 },
                                 [=](const QString &error_msg)
                                 { emit downloadError(error_msg); });
    git_downloader->downloadFile(url);
}

void UpdateManager::downloadPackage(const QString &url)
{
    git_downloader->resetCallbacks();
    git_downloader->setCallbacks([=](quint64 n1, quint64 n2)
                                 { emit downloadProgress(n1, n2); },
                                 [=](QByteArray data)
                                 {
                                     // 默认将安装包保存到当前设置的缓存目录
                                     QFile file(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir) + "update.tar.gz");
                                     if (!file.open(QIODevice::WriteOnly))
                                     {
                                         LOG_ERROR(file.errorString().toStdString());
                                         return;
                                     }
                                     file.write(data);
                                     file.flush();
                                     file.close();

                                     emit downloadPackageDone(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir) + "update.tar.gz");
                                 },
                                 [=](const QString &error_msg)
                                 { emit downloadError(error_msg); });
    git_downloader->downloadFile(url);
}

VersionInfo UpdateManager::VersionParser::parse(QByteArray version_json)
{
    QJsonDocument doc = QJsonDocument::fromJson(version_json);
    if (doc.isNull())
    {
        LOG_ERROR("JSON解析失败");
        return {};
    }

    QJsonObject root_obj = doc.object();
    if (root_obj.isEmpty())
    {
        LOG_ERROR("JSON对象为空");
        return {};
    }

    VersionInfo version_info{};

    version_info.lastest_version = root_obj["latest_version"].toString();
    version_info.update_time = root_obj["update_time"].toString();

    QJsonObject versions_obj = root_obj["versions"].toObject();
    if (!versions_obj.contains(version_info.lastest_version))
    {
        LOG_ERROR("最新版本不存在:" << version_info.lastest_version.toStdString());
        return version_info;
    }

    QJsonObject lastest_version_obj = versions_obj[version_info.lastest_version].toObject();

    QStringList changelog_list;
    QJsonArray changelog_array = lastest_version_obj["changelog"].toArray();
    for (const QJsonValue &item : changelog_array)
    {
        changelog_list.append(item.toString());
    }
    version_info.changelog = changelog_list.join("\n");

    QJsonObject download_url_obj = lastest_version_obj["downloads"].toObject();

    if (download_url_obj.contains("windows"))
    {
        QJsonObject windows_obj = download_url_obj["windows"].toObject();
        version_info.win_github_url = windows_obj["github_url"].toString();
        version_info.win_gitee_url = windows_obj["gitee_url"].toString();
    }
    else
    {
        LOG_ERROR("未找到Windows下载链接");
    }

    if (download_url_obj.contains("linux"))
    {
        QJsonObject linux_obj = download_url_obj["linux"].toObject();
        version_info.linux_github_url = linux_obj["github_url"].toString();
        version_info.linux_gitee_url = linux_obj["gitee_url"].toString();
    }
    else
    {
        LOG_ERROR("未找到Linux下载链接");
    }

    return version_info;
}

UpdateManager::GitDownloader::GitDownloader(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this)), timeout_timer(new QTimer(this))
{
    timeout_timer->setSingleShot(true);
    connect(timeout_timer, &QTimer::timeout, this, &GitDownloader::onTimeout);
}

void UpdateManager::GitDownloader::downloadFile(QString url)
{
    if (current_reply)
    {
        cancelDownload();
    }

    if (url.isEmpty())
    {
        error_cb("不支持的平台或无效的URL");
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Qt Downloader)");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    current_reply = manager->get(request);
    connect(current_reply, &QNetworkReply::finished, this, &GitDownloader::onReplyFinished);
    connect(current_reply, &QNetworkReply::downloadProgress,
            this, [=](quint64 n1, quint64 n2)
            { 
                if(n1 > 0 && timeout_timer->isActive())
                {
                    timeout_timer->stop();
                }
                progress_cb(n1, n2); });

    timeout_timer->start(20000); // 20秒超时
}

void UpdateManager::GitDownloader::setCallbacks(DownloadProgressCallback progressCb,
                                                DownloadFinishedCallback finishedCb,
                                                DownloadErrorCallback errorCb)
{
    progress_cb = progressCb;
    finished_cb = finishedCb;
    error_cb = errorCb;
}

void UpdateManager::GitDownloader::resetCallbacks()
{
    progress_cb = nullptr;
    finished_cb = nullptr;
    error_cb = nullptr;
}

void UpdateManager::GitDownloader::cancelDownload()
{
    if (current_reply && current_reply->isRunning())
    {
        current_reply->abort();
        current_reply = nullptr;
        timeout_timer->stop();
    }
}

void UpdateManager::GitDownloader::onReplyFinished()
{
    timeout_timer->stop();

    if (!current_reply)
    {
        return;
    }

    if (current_reply->error() == QNetworkReply::NoError)
    {
        // 检查重定向
        QVariant redirectAttr = current_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (!redirectAttr.isNull())
        {
            QUrl redirectUrl = redirectAttr.toUrl();
            if (redirectUrl.isRelative())
            {
                redirectUrl = current_reply->url().resolved(redirectUrl);
            }
            handleRedirect(redirectUrl);
            current_reply->deleteLater();
            current_reply = nullptr;
            return;
        }

        // 读取数据
        QByteArray data = current_reply->readAll();

        finished_cb(data);
    }
    else if (current_reply->error() != QNetworkReply::OperationCanceledError)
    {
        error_cb(QString("下载失败: %1").arg(current_reply->errorString()));
    }

    current_reply->deleteLater();
    current_reply = nullptr;
}

void UpdateManager::GitDownloader::onTimeout()
{
    if (current_reply && current_reply->isRunning())
    {
        current_reply->abort();
        error_cb("请求超时 (20秒)");
    }
}

void UpdateManager::GitDownloader::handleRedirect(const QUrl &redirect_url)
{
    QNetworkRequest request(redirect_url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Qt Downloader)");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    current_reply = manager->get(request);
    current_reply->disconnect();
    connect(GitDownloader::current_reply, &QNetworkReply::finished, this, &GitDownloader::onReplyFinished);
    connect(current_reply, &QNetworkReply::downloadProgress,
            this, [=](quint64 n1, quint64 n2)
            {                 
                if(n1 > 0)
                {
                    timeout_timer->stop();
                }
                progress_cb(n1, n2); });

    timeout_timer->start(30000);
}
