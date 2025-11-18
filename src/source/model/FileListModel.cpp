#include "model/FileListModel.h"
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

FileInfo::idType FileInfo::current_type = FileInfo::idType::LOW;
quint32 FileInfo::file_id_counter = 0;

FileListModel::FileListModel(QObject* parent) :
    QAbstractListModel(parent)
{
    //FileInfo默认为LOW
    FileInfo::setIdBegin(FileInfo::idType::LOW);
}

FileListModel::~FileListModel()
{
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : file_list.size();
}

QVariant FileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= file_list.size())
        return QVariant();

    const FileInfo& file = file_list.at(index.row());

    switch (role) {
    case Qt::ToolTipRole:
        return file.file_name + "\n路径: " + file.source_path + "\n大小: " + file.format_file_size;
    case FileNameRole:
        return file.file_name;
    case FileSourcePathRole:
        return file.source_path;
    case FileUrlRole:
        return file.file_url;
    case FileSizeRole:
        return file.file_size;
    case FileIconRole:
        return file.icon;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileListModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        {FileNameRole, "fileName"},
        {FileSourcePathRole, "filePath"},
        {FileUrlRole, "fileUrl"},
        {FileSizeRole, "fileSize"},
        {FileIconRole, "fileIcon"},
        {Qt::ToolTipRole, "toolTip"}
    };
    return roles;
}

void FileListModel::addFiles(const QList<QString>& files, bool is_remote_file)
{
    if (files.isEmpty())
        return;

    QList<FileInfo> unique_files;
    for (const QString& file : files) 
    {
        if(!isFileExists(file)) {
            unique_files.append(FileInfo(is_remote_file, file));
        }
    }

    //只有存在新文件时才插入
    if (!unique_files.isEmpty()) {
        beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + unique_files.size() - 1);
        file_list.append(unique_files);
        endInsertRows();
    }

    qDebug() << "文件添加完成，当前文件数:" << file_list.size();
}

bool FileListModel::isFileExists(const QString& filePath)
{
    return std::any_of(file_list.begin(), file_list.end(),
                      [&](const FileInfo& info) {
                          return info.file_url == filePath;
                      });
}

void FileListModel::removeFile(int index)
{
    if (index < 0 || index >= file_list.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    file_list.removeAt(index);
    endRemoveRows();
}

void FileListModel::clearAll()
{
    if (file_list.isEmpty())
        return;

    beginResetModel();
    file_list.clear();
    endResetModel();
}

int FileListModel::getFileCount() const
{
    return file_list.size();
}