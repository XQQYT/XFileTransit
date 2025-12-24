#ifndef _FILELISTMODEL_H
#define _FILELISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include "model/FileIconManager.h"
#include "driver/impl/FileUtility.h"
#include "control/GlobalStatusManager.h"

#include <iostream>

struct FileInfo;
class FileListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  static inline const uint32_t auto_download_file_size = 50 * 1024 * 1024; // 50MB
  enum FileStatus
  {
    StatusPending = 0,       // 等待
    StatusLocalDefault,      // 本地文件默认
    StatusRemoteDefault,     // 远程文件默认
    StatusUploading,         // 上传中
    StatusDownloading,       // 下载中
    StatusUploadCompleted,   // 上传完成
    StatusDownloadCompleted, // 下载完成
    StatusError,             // 错误
    StatusUploadCancel,      // 取消上传
    StatusDownloadCancel     // 取消下载
  };
  Q_ENUM(FileStatus)

  enum Roles
  {
    FileNameRole = Qt::UserRole + 1,
    FileSourcePathRole,
    FileUrlRole,
    FileSizeRole,
    FileIconRole,
    FileStatusRole,
    isRemoteRole,
    FileProgressRole,
    FileSpeedRole
  };

  explicit FileListModel(QObject *parent = nullptr);
  ~FileListModel();

  // QAbstractItemModel接口实现
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void addFiles(const QList<QString> &files, bool is_remote_file);
  Q_INVOKABLE void removeFile(int index);
  Q_INVOKABLE void clearAll();
  Q_INVOKABLE int getFileCount() const;
  Q_INVOKABLE void syncCurrentFiles();
  Q_INVOKABLE void updateFilesId();
  Q_INVOKABLE void copyText(const QString &text);
  Q_INVOKABLE void downloadFile(int index);
  Q_INVOKABLE void cleanTmpFiles();
  Q_INVOKABLE bool isTransferring();
  Q_INVOKABLE void cancelTransit(int index);
  Q_INVOKABLE void cancelAllTransit();
  Q_INVOKABLE void downloadAll();
  void updateFilePath(QString new_path);
  void setAutoDownload(bool enable);
  void addRemoteFiles(std::vector<std::vector<std::string>> files);
  void haveDownLoadRequest(std::vector<std::string> file_ids);
signals:
  void themeChanged(int theme);
  void mainWinExpand();
public slots:
  void onConnectionClosed();
  void onSettingsChanged(Settings::Item item, QVariant value);

private:
  bool isFileExists(const QString &filePath);
  void removeAllRemoteFiles();
  void onHaveExpiredFile(std::vector<std::string> id);
  void deleteFile(int index);
  void removeFileById(std::vector<std::string> id);
  void onUploadFileProgress(uint32_t id, uint8_t progress, uint32_t speed, bool is_end);
  void onDownLoadProgress(uint32_t id, uint8_t progress, uint32_t speed, bool is_end);
  void onHaveCancelFile(uint32_t id);
  std::pair<int, FileInfo &> findFileInfoById(uint32_t id);
  bool auto_expand;
  bool auto_clear_cache;

private:
  QList<FileInfo> file_list;
  QHash<uint32_t, QVector<uint32_t>> speed_history;
  bool auto_download{true};
};

struct FileInfo
{
public:
  quint32 id;
  bool is_remote_file;
  bool is_folder;
  QString file_name;
  QString source_path; // is_remote_file为false是才有效
  QUrl file_url;       // is_remote_file为false是才有效
  quint64 file_size;
  QString format_file_size;
  QUrl icon;
  uint8_t file_status;
  quint8 progress;
  quint32 speed;

public:
  FileInfo() = default;

  static QString getFileName(const QString &file_url)
  {
    int last_separator_index = file_url.lastIndexOf('/');
    if (last_separator_index == -1)
      last_separator_index = file_url.lastIndexOf('\\');
    return file_url.mid(last_separator_index + 1);
  }

  static QString getFilePath(const QString &file_url)
  {
    if (file_url.startsWith("file:///"))
    {
#ifdef Q_OS_WIN
      return QUrl(file_url).toLocalFile();
#else
      return file_url.mid(7);
#endif
    }
    return file_url;
  }

  static QString getFileSuffix(const QString &fileName)
  {
    int last_dot = fileName.lastIndexOf('.');
    if (last_dot != -1 && last_dot < fileName.length() - 1)
    {
      return fileName.mid(last_dot + 1).toLower();
    }
    return "";
  }
  // 一般用于本地文件构造
  FileInfo(const bool irf, const QString &url, const quint32 file_id = 0, const quint64 size = 0, const QString &fn = QString())
      : is_remote_file(irf), file_url(url), file_status(FileListModel::FileStatus::StatusLocalDefault)
  {
    if (irf)
    {
      throw std::runtime_error("Don'n use this Constructor to construct locate file");
    }

    // 不是远程文件时，从本地获取文件信息
    id = GlobalStatusManager::getInstance().getFileId();
    file_name = getFileName(url);
    source_path = getFilePath(url);
    is_folder = FileSystemUtils::isDirectory(source_path.toStdString());
    file_url = url;
    if (is_folder)
    {
      file_size = FileSystemUtils::calculateFolderSize(source_path.toStdString());
    }
    else
    {
      file_size = FileSystemUtils::getFileSize(source_path.toStdString());
    }
    format_file_size = QString::fromStdString(FileSystemUtils::formatFileSize(file_size));
    icon = FileIconManager::getInstance().getFileIcon(url, is_folder);
    progress = 0;
  }

  // 一般用于远程文件构建
  FileInfo(const bool irf, const quint32 file_id, const bool is_folder, const QString fn, quint64 fs)
      : is_remote_file(irf), id(file_id), is_folder(is_folder), file_name(fn), file_size(fs),
        file_status(FileListModel::FileStatus::StatusRemoteDefault)
  {
    if (!irf)
    {
      throw std::runtime_error("Don'n use this Constructor to construct remote file");
    }
    file_url = QUrl::fromLocalFile(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir) + file_name);
    source_path = QString::fromStdString(GlobalStatusManager::absolute_tmp_dir) + file_name;
    icon = FileIconManager::getInstance().getFileIconBySuffix(getFileSuffix(file_name), is_folder);
    format_file_size = QString::fromStdString(FileSystemUtils::formatFileSize(file_size));
    progress = 0;
  }
};

#endif // FILELISTMODEL_H