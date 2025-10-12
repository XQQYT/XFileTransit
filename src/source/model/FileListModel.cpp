#include "model/FileListModel.h"
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

QString FileInfo::formatFileSize(quint64 bytes)
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

FileListModel::FileListModel(QObject* parent) :
    QAbstractListModel(parent)
{
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

QString FileListModel::getFileName(const QString& file_url)
{
    int last_separator_index = file_url.lastIndexOf('/');
    if (last_separator_index == -1)
        last_separator_index = file_url.lastIndexOf('\\');
    return file_url.mid(last_separator_index + 1);
}

QString FileListModel::getFilePath(const QString& file_url)
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

quint64 FileListModel::getFileSize(const QString& file_url)
{
    QString file_path = getFilePath(file_url);
    QFileInfo file_info(file_path);
    if (!file_info.exists()) {
        qDebug() << "文件不存在:" << file_path;
        return 0;
    }
    return file_info.size();
}

void FileListModel::addFiles(const QList<QString>& files, bool is_remote_file)
{
    if (files.isEmpty())
        return;

    beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + files.size() - 1);

    for (const QString& file : files) {
        QString fileName = getFileName(file);
        QString filePath = getFilePath(file);
        quint64 fileSize = getFileSize(file);

        qDebug() << "添加文件:" << fileName << "路径:" << filePath << "大小:" << fileSize;

        file_list.append(FileInfo(is_remote_file, fileName, filePath, file, fileSize));
    }

    endInsertRows();

    qDebug() << "文件添加完成，当前文件数:" << file_list.size();
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