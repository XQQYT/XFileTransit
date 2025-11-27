#include "model/FileListModel.h"
#include "model/ModelManager.h"
#include "control/EventBusManager.h"
#include "control/GlobalStatusManager.h"
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

FileListModel::FileListModel(QObject* parent) :
    QAbstractListModel(parent)
{
    //FileInfo默认为LOW
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);

    EventBusManager::instance().subscribe("/sync/have_addfiles",
        std::bind(&FileListModel::addRemoteFiles,
            this,
            std::placeholders::_1));
    EventBusManager::instance().subscribe("/sync/have_deletefiles",
        std::bind(&FileListModel::removeFileById,
            this,
            std::placeholders::_1));
    EventBusManager::instance().subscribe("/file/have_download_request",
        std::bind(&FileListModel::haveDownLoadRequest,
            this,
            std::placeholders::_1));
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
        return file.file_name + "\n路径: " + (file.is_remote_file ? "remote file" : file.source_path) + "\n大小: " + file.format_file_size;
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
    case FileStatusRole:
        return file.file_status;
    case isRemoteRole:
        return file.is_remote_file;
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
        {FileStatusRole, "fileStatus"},
        {isRemoteRole, "isRemote"},
        {Qt::ToolTipRole, "toolTip"}
    };
    return roles;
}

//添加本地文件
void FileListModel::addFiles(const QList<QString>& files, bool is_remote_file)
{
    if (files.isEmpty())
        return;

    QVector<FileInfo> unique_files;
    std::vector<std::string> files_to_send;
    for (const QString& file : files)
    {
        if (!isFileExists(file)) {
            FileInfo cur_file(is_remote_file, file);
            //步长为3
            if (GlobalStatusManager::getInstance().getConnectStatus())
            {
                files_to_send.push_back(std::to_string(cur_file.id));
                files_to_send.push_back(std::to_string(cur_file.is_folder));
                files_to_send.push_back(cur_file.file_name.toStdString());
                files_to_send.push_back(cur_file.format_file_size.toStdString());
            }
            unique_files.append(std::move(cur_file));
        }
    }

    //只有存在新文件时才插入
    if (!unique_files.isEmpty()) {
        beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + unique_files.size() - 1);
        file_list.append(unique_files);
        endInsertRows();
    }

    if (GlobalStatusManager::getInstance().getConnectStatus() && !files_to_send.empty())
    {
        EventBusManager::instance().publish("/sync/send_addfiles", files_to_send, uint8_t(4));
    }
}

void FileListModel::addRemoteFiles(std::vector<std::vector<std::string>> files)
{
    QVector<FileInfo> remote_files;
    for (const auto& file : files)
    {
        remote_files.append(FileInfo(true,
            std::stoul(file[0]), std::stoi(file[1]) != 0,
            QString::fromStdString(file[2]), QString::fromStdString(file[3])));
    }

    if (!remote_files.empty())
    {
        beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + remote_files.size() - 1);
        file_list.append(remote_files);
        endInsertRows();
    }
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

    deleteFile(index);
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

void FileListModel::onConnectionClosed()
{
    removeAllRemoteFiles();
}

void FileListModel::removeAllRemoteFiles()
{
    beginResetModel();
    file_list.erase(std::remove_if(file_list.begin(), file_list.end(),
                          [](const FileInfo& info) { return info.is_remote_file; }),
           file_list.end());
    endResetModel();
}

void FileListModel::updateFilesId()
{
    for(auto& file : file_list)
    {
        file.id = GlobalStatusManager::getInstance().getFileId();
    }
}

void FileListModel::syncCurrentFiles()
{
    std::vector<std::string> files_to_send;
    files_to_send.reserve(file_list.size());
    for(auto& cur_file: file_list)
    {
        if(cur_file.is_remote_file) continue;
        //先更新当前所有文件的id
        cur_file.id = GlobalStatusManager::getInstance().getFileId();

        files_to_send.push_back(std::to_string(cur_file.id));
        files_to_send.push_back(std::to_string(cur_file.is_folder));
        files_to_send.push_back(cur_file.file_name.toStdString());
        files_to_send.push_back(cur_file.format_file_size.toStdString());
    }
    if (GlobalStatusManager::getInstance().getConnectStatus() && !files_to_send.empty())
    {
        EventBusManager::instance().publish("/sync/send_addfiles", files_to_send, uint8_t(4));
    }
}

void FileListModel::copyText(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
}

void FileListModel::deleteFile(int index)
{
    quint32 file_id = file_list[index].id;
    EventBusManager::instance().publish("/sync/send_deletefiles", static_cast<uint32_t>(file_id));
}

void FileListModel::removeFileById(std::vector<std::string> id)
{
    beginResetModel();
    for(auto file : id)
    {
        uint32_t target_id = std::stoul(file);
            file_list.erase(std::remove_if(file_list.begin(), file_list.end(),
                          [&target_id](const FileInfo& info) { return info.id == target_id; }),
           file_list.end());
    }
    endResetModel();
}

void FileListModel::downloadFile(int index)
{
    auto target_file = file_list[index];
    EventBusManager::instance().publish("/file/send_get_file", uint32_t(target_file.id));
}

void FileListModel::haveDownLoadRequest(std::vector<std::string> file_ids)
{
    for(auto id : file_ids)
    {
        uint32_t target_id = std::stoul(id);
        file_list[target_id].file_status = FileStatus::StatusPending;
        EventBusManager::instance().publish("/file/have_file_to_send", target_id, file_list[target_id].source_path.toStdString());
    }
}