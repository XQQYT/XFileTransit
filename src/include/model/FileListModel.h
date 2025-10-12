#ifndef _FILELISTMODEL_H
#define _FILELISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QIcon>

struct FileInfo
{
  QString file_name;
  QString source_path;
  QString file_url;
  quint64 file_size;
  QIcon icon;

  FileInfo(const QString& name, const QString& path, const QString& url, quint64 size, const QIcon& ico = QIcon())
    : file_name(name), source_path(path), file_url(url), file_size(size), icon(ico) {
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

  // QAbstractItemModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // 自定义方法
  Q_INVOKABLE void addFiles(const QList<QString>& files);
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