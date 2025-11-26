#ifndef _FILELISTMODEL_H
#define _FILELISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include "model/FileIconManager.h"
#include "control/GlobalStatusManager.h"

#include <iostream>

struct FileInfo
{
public:
  quint32 id;
  bool is_remote_file;
  bool is_folder;
  QString file_name;
  QString source_path;  //is_remote_file为false是才有效
  QString file_url;   //is_remote_file为false是才有效
  quint64 file_size;
  QString format_file_size;
  QUrl icon;
  quint8 file_status;

public:
  FileInfo() = default;

  static QString getFileName(const QString& file_url)
  {
    int last_separator_index = file_url.lastIndexOf('/');
    if (last_separator_index == -1)
      last_separator_index = file_url.lastIndexOf('\\');
    return file_url.mid(last_separator_index + 1);
  }

  static QString getFilePath(const QString& file_url)
  {
    if (file_url.startsWith("file:///")) {
#ifdef Q_OS_WIN
      return QUrl(file_url).toLocalFile();
#else
      return file_url.mid(7);
#endif
    }
    return file_url;
  }

  static quint64 getFileSize(const QString& file_url)
  {
    QString file_path = getFilePath(file_url);
    QFile file(file_path);
    if (!file.exists()) {
      qDebug() << "文件不存在:" << file_path;
      return 0;
    }
    return file.size();
  }

  //TODO
  //改为生产消费模型 ，防止大文件卡死
  static quint64 getFolderSize(const QString& folderPath)
  {
    quint64 totalSize = 0;

    // 递归遍历所有文件和目录
    QDirIterator it(folderPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
      it.next();
      totalSize += it.fileInfo().size();
    }

    return totalSize;
  }

  static QString formatFileSize(quint64 bytes)
  {
    const char* suffixes[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    double size = static_cast<double>(bytes);
    int i = 0;

    while (size >= 1024 && i < 5) {
      size /= 1024;
      ++i;
    }

    // 保留两位小数（当数值大于1KB时）
    if (i == 0)
      return QString::number(static_cast<qulonglong>(size)) + " " + suffixes[i];
    else
      return QString("%1 %2").arg(QString::number(size, 'f', 2)).arg(suffixes[i]);
  }
  static bool isDirectoryWithQDir(const QString& filePath)
  {
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isDir();
  }
  static QString getFileSuffix(const QString& fileName)
  {
    int last_dot = fileName.lastIndexOf('.');
    if (last_dot != -1 && last_dot < fileName.length() - 1) {
      return fileName.mid(last_dot + 1).toLower();
    }
    return "";
  }
  //一般用于本地文件构造
  FileInfo(const bool irf, const QString& url, const quint32 file_id = 0, const quint64 size = 0, const QString& fn = QString())
    : is_remote_file(irf), file_url(url), file_status(0)
  {
    if (irf)
    {
      throw std::runtime_error("Don'n use this Constructor to construct locate file");
    }

    //不是远程文件时，从本地获取文件信息
    id = GlobalStatusManager::getInstance().getFileId();
    file_name = getFileName(url);
    source_path = getFilePath(url);
    is_folder = isDirectoryWithQDir(source_path);
    file_url = url;
    if (is_folder)
    {
      file_size = getFolderSize(source_path);
    }
    else
    {
      file_size = getFileSize(url);
    }
    format_file_size = formatFileSize(file_size);
    icon = FileIconManager::getInstance().getFileIcon(url, is_folder);
    file_status = 3; //StatusCompleted
  }

  //一般用于远程文件构建
  FileInfo(const bool irf, const quint32 file_id, const bool is_folder, const QString fn, const QString ffs)
    : is_remote_file(irf), id(file_id), is_folder(this->is_folder), file_name(fn), format_file_size(ffs), file_status(0)
  {
    if (!irf)
    {
      throw std::runtime_error("Don'n use this Constructor to construct remote file");
    }
    icon = FileIconManager::getInstance().getFileIconBySuffix(getFileSuffix(file_name), is_folder);
    file_status = 1;  //StatusPending
  }
};

class FileListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  enum FileStatus {
    StatusPending = 0,     // 等待
    StatusDefault,
    StatusUploading,       // 上传中
    StatusDownloading,     // 下载中  
    StatusCompleted,       // 完成
    StatusError           // 错误
  };
  Q_ENUM(FileStatus)

    enum Roles {
    FileNameRole = Qt::UserRole + 1,
    FileSourcePathRole,
    FileUrlRole,
    FileSizeRole,
    FileIconRole,
    FileStatusRole,
    isRemoteRole
  };

  explicit FileListModel(QObject* parent = nullptr);
  ~FileListModel();

  // QAbstractItemModel接口实现
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void addFiles(const QList<QString>& files, bool is_remote_file);
  Q_INVOKABLE void removeFile(int index);
  Q_INVOKABLE void clearAll();
  Q_INVOKABLE int getFileCount() const;
  Q_INVOKABLE void syncCurrentFiles();
  Q_INVOKABLE void updateFilesId();
  Q_INVOKABLE void copyText(const QString &text);
  void addRemoteFiles(std::vector<std::vector<std::string>> files);

public slots:
  void onConnectionClosed();
private:
  bool isFileExists(const QString& filePath);
  void removeAllRemoteFiles();
  void deleteFile(int index);
  void removeFileById(std::vector<std::string> id);
private:
  QList<FileInfo> file_list;
};

#endif // FILELISTMODEL_H