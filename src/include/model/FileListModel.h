#ifndef _FILELISTMODEL_H
#define _FILELISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include "model/FileIconManager.h"

struct FileInfo
{
  bool is_remote_file;
  QString file_name;
  QString source_path;
  QString file_url;
  quint64 file_size;
  QString format_file_size;
  QString icon;
  QString formatFileSize(quint64 bytes);
  FileInfo(const bool irf, const QString& name, const QString& path, const QString& url, quint64 size, const QString& ico = QString())
    : is_remote_file(irf), file_name(name), source_path(path), file_url(url), file_size(size), icon(ico)
  {
    icon = FileIconManager::getInstance().getFileIconSvg(
      file_url, is_remote_file);
    format_file_size = formatFileSize(file_size);
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