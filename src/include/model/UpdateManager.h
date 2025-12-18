#ifndef _UPDATEMANAGER_H
#define _UPDATEMANAGER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QTimer>
#include <QtCore/QByteArray>
#include <QtCore/QString>

enum class GitPlatform
{
    Gitee,
    Github
};
struct VersionInfo
{
    QString lastest_version;
    QString update_time;
    QString changelog;
    QString win_url;
    QString linux_url;
};
class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    void downloadVersionJson(const GitPlatform platform, const QString &owner,
                             const QString &repo, const QString &branch,
                             const QString &file_path);
    void downloadPackage(const VersionInfo &new_version_info);
    QString buildUrl(const GitPlatform platform, const QString &owner,
                     const QString &repo, const QString &branch,
                     const QString &file_path) const;

private:
    class GitDownloader;
    class VersionParser;
signals:
    void downloadProgress(qint64, qint64);
    void downloadFinished(const QByteArray &);
    void downloadError(const QString &);

    void versionJsonParsedDone(VersionInfo info);
    void downloadPackageDone(QString save_path);

private:
    std::unique_ptr<GitDownloader> git_downloader;
    std::unique_ptr<VersionParser> version_parser;
};

class UpdateManager::GitDownloader : public QObject
{
    Q_OBJECT

public:
    explicit GitDownloader(QObject *parent = nullptr);

    void downloadFile(QString url);
    using DownloadProgressCallback = std::function<void(qint64, qint64)>;
    using DownloadFinishedCallback = std::function<void(const QByteArray &)>;
    using DownloadErrorCallback = std::function<void(const QString &)>;

    void setCallbacks(DownloadProgressCallback progressCb,
                      DownloadFinishedCallback finishedCb,
                      DownloadErrorCallback errorCb);
    void resetCallbacks();
    void cancelDownload();

    bool isDownloading() const { return current_reply != nullptr; }

private slots:
    void onReplyFinished();
    void onTimeout();

private:
    QNetworkAccessManager *manager;
    QNetworkReply *current_reply = nullptr;
    QTimer *timeout_timer;

    void handleRedirect(const QUrl &redirect_url);

    DownloadProgressCallback progress_cb;
    DownloadFinishedCallback finished_cb;
    DownloadErrorCallback error_cb;
};

class UpdateManager::VersionParser
{
public:
    VersionInfo parse(QByteArray version_json);
};

#endif