#ifndef _FILELISTMODEL_H
#define _FILELISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtCore/QDir>
#include "model/FileIconManager.h"

struct FileInfo
{
  enum class idType{
    LOW,
    HIGH,
    UNDEFINED
  };
  static quint32 file_id_counter;
  static idType current_type;
  quint32 id;
  bool is_remote_file;
  bool is_folder;
  QString file_name;
  QString source_path;  //is_remote_file为false是才有效
  QString file_url;   //is_remote_file为false是才有效
  quint64 file_size;
  QString format_file_size;
  QUrl icon;

  static void setIdBegin(idType type)
  {
    current_type = type;
    switch (type)
    {
    case idType::LOW:
      file_id_counter = 0;
      break;
    case idType::HIGH:
      file_id_counter = UINT32_MAX;
      break;
    default:
      break;
    }
  }

  static void reset()
  {
    current_type = idType::UNDEFINED;
  }

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
      QDir dir(filePath);
      return dir.exists();
  }
  FileInfo(const bool irf, const QString& url, const quint32 file_id = 0, const quint64 size = 0, const QString& fn = QString())
    : is_remote_file(irf), file_url(url)
  {
    if(current_type == idType::UNDEFINED)
    {
      throw std::runtime_error("Please set id begin");
    }
    //不是远程文件时，从本地获取文件信息
    if(!is_remote_file)
    {
      id = (current_type == idType::LOW) ? file_id_counter++ : file_id_counter--; 
      file_name = getFileName(url);
      source_path = getFilePath(url);
      file_url = url;
      file_size = getFileSize(url);
      format_file_size = formatFileSize(file_size);
      icon = FileIconManager::getInstance().getFileIcon(url, isDirectoryWithQDir(source_path));
      qDebug()<<icon<<source_path;
    }
    else//远程文件时只需要id,file name, file size即可
    {
      id = file_id;
      file_name = fn;
      file_size = size;
    }
  }
};

class FileListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  enum Roles {
    FileNameRole = Qt::UserRole + 1,
    FileSourcePathRole,
    FileUrlRole,
    FileSizeRole,
    FileIconRole
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

private:
  QString getFileName(const QString& file_url);
  QString getFilePath(const QString& file_url);
  quint64 getFileSize(const QString& file_url);
private:
  QList<FileInfo> file_list;
};

#endif // FILELISTMODEL_H