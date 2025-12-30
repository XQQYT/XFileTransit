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
    GitCode,
    Github
};
struct VersionInfo
{
    QString lastest_version;
    QString zh_changelog;
    QString en_changelog;
    QString release_date;
    QString win_github_url;
    QString linux_github_url;
    QString win_gitcode_url;
    QString linux_gitcode_url;
};
class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    void downloadVersionJson(const GitPlatform platform, const QString &owner,
                             const QString &repo, const QString &branch,
                             const QString &file_path);
    void setProxy(const QString address, const QString port, const QString username, const QString password);
    void removeProxy();
    bool haveProxy();
    void downloadPackage(const QString &url);
    QString buildUrl(const GitPlatform platform, const QString &owner,
                     const QString &repo, const QString &branch,
                     const QString &file_path) const;
    void cancelDownload();
    void testProxy(const QString address, const QString port, const QString username, const QString password);
    void cancelTestProxy();

private:
    class GitDownloader;
    class VersionParser;
    class ProxyTester;
signals:
    void downloadProgress(qint64, qint64);
    void downloadFinished(const QByteArray &);
    void downloadError(const QString &);

    void versionJsonParsedDone(VersionInfo info);
    void downloadPackageDone(QString save_path);

    void testResult(bool ret);
    void testError(const QString error);

private:
    std::unique_ptr<GitDownloader> git_downloader;
    std::unique_ptr<VersionParser> version_parser;
    std::unique_ptr<ProxyTester> proxy_tester;
};

class UpdateManager::GitDownloader : public QObject
{
    Q_OBJECT

public:
    explicit GitDownloader(QObject *parent = nullptr);

    void setProxy(const QString address, const QString port, const QString username, const QString password);
    void removeProxy();
    bool haveProxy();
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

class UpdateManager::ProxyTester : public QObject
{
    Q_OBJECT

public:
    explicit ProxyTester(QObject *parent = nullptr);
    void setCb(std::function<void(QString)> ecb, std::function<void(bool)> rcb);
    void testProxy(const QString address, const QString port, const QString username, const QString password);
    void cancelTestProxy();
private slots:
    void onFinished();
    void onSslErrors(const QList<QSslError> &errors);
    void onErrorOccurred(QNetworkReply::NetworkError);

private:
    std::function<void(QString)> error_cb;
    std::function<void(bool)> result_cb;
    QTimer *timeout_timer;
    QNetworkReply *current_reply;
    bool ret{true};
};

#endif