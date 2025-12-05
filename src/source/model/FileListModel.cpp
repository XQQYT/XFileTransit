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
    EventBusManager::instance().subscribe("/file/upload_progress",
        std::bind(&FileListModel::onUploadFileProgress,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
    EventBusManager::instance().subscribe("/file/download_progress",
        std::bind(&FileListModel::onDownLoadProgress,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
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
    case FileProgressRole:
        return file.progress;
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
        {FileProgressRole, "fileProgress"},
        {Qt::ToolTipRole, "toolTip"}
    };
    return roles;
}

std::pair<int, FileInfo&> FileListModel::findFileInfoById(uint32_t id)
{
    auto it = std::find_if(file_list.begin(), file_list.end(),
        [id](const FileInfo& info) {
            return info.id == id;
        });
    if (it == file_list.end()) {
        throw std::runtime_error("FileInfo with id " + std::to_string(id) + " not found");
    }
    int index = std::distance(file_list.begin(), it);
    return std::pair<int, FileInfo&>(index, *it);
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
                files_to_send.push_back(cur_file.file_name.toUtf8().constData());
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
        GlobalStatusManager::getInstance().insertFile(remote_files.back().id, remote_files.back().file_name.toUtf8().constData());
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

    if(GlobalStatusManager::getInstance().getConnectStatus())
    {
        deleteFile(index);
    }
    GlobalStatusManager::getInstance().removeFile(file_list[index].id);
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
    for(auto it = file_list.begin(); it < file_list.end(); it++)
    {
        if(it->is_remote_file)
        {
            file_list.erase(it);
            continue;
        }
        it->file_status = FileStatus::StatusLocalDefault;
    }
    endResetModel();
}

void FileListModel::updateFilesId()
{
    for (auto& file : file_list)
    {
        file.id = GlobalStatusManager::getInstance().getFileId();
    }
}

void FileListModel::syncCurrentFiles()
{
    std::vector<std::string> files_to_send;
    files_to_send.reserve(file_list.size());
    for (auto& cur_file : file_list)
    {
        if (cur_file.is_remote_file) continue;
        //先更新当前所有文件的id
        cur_file.id = GlobalStatusManager::getInstance().getFileId();

        files_to_send.push_back(std::to_string(cur_file.id));
        files_to_send.push_back(std::to_string(cur_file.is_folder));
        files_to_send.push_back(cur_file.file_name.toUtf8().constData());
        files_to_send.push_back(cur_file.format_file_size.toStdString());
    }
    if (GlobalStatusManager::getInstance().getConnectStatus() && !files_to_send.empty())
    {
        EventBusManager::instance().publish("/sync/send_addfiles", files_to_send, uint8_t(4));
    }
}

void FileListModel::copyText(const QString& text)
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
}

void FileListModel::deleteFile(int index)
{
    quint32 file_id = file_list[index].id;
    EventBusManager::instance().publish("/sync/send_deletefiles", static_cast<uint32_t>(file_id));
}

void FileListModel::removeFileById(std::vector<std::string> id)
{
    if (id.empty()) return;
    std::vector<int> indicesToRemove;
    for (const auto& fileIdStr : id) {
        uint32_t target_id = std::stoul(fileIdStr);
        for (int i = 0; i < file_list.size(); ++i) {
            if (file_list[i].id == target_id) {
                indicesToRemove.push_back(i);
                break;
            }
        }
    }
    if (indicesToRemove.empty()) return;
    std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
    for (int index : indicesToRemove) {
        if (index >= 0 && index < file_list.size()) {
            deleteFile(index);
            beginRemoveRows(QModelIndex(), index, index);
            file_list.removeAt(index);
            endRemoveRows();
        }
    }
}
void FileListModel::downloadFile(int i)
{
    EventBusManager::instance().publish("/file/send_get_file", uint32_t(file_list[i].id));

    file_list[i].file_status = FileStatus::StatusPending;

    QModelIndex model_index = index(i, 0);
    QVector<int> roles = { FileStatusRole };

    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::haveDownLoadRequest(std::vector<std::string> file_ids)
{
    for (auto id : file_ids)
    {
        uint32_t target_id = std::stoul(id);
        std::cout << "haveDownLoadRequest " << target_id << std::endl;
        auto target_file = findFileInfoById(target_id);
        target_file.second.file_status = FileStatus::StatusPending;
        EventBusManager::instance().publish("/file/have_file_to_send", target_id, target_file.second.source_path.toStdString());

        target_file.second.file_status = FileStatus::StatusPending;

        QModelIndex model_index = index(target_file.first, 0);
        QVector<int> roles = { FileStatusRole };

        emit dataChanged(model_index, model_index, roles);
    }
}

void FileListModel::onUploadFileProgress(uint32_t id, uint8_t progress, bool is_end)
{
    auto target_file = findFileInfoById(id);

    target_file.second.file_status = is_end ? FileStatus::StatusUploadCompleted : FileStatus::StatusUploading;
    target_file.second.progress = static_cast<int>(progress);

    QModelIndex model_index = index(target_file.first, 0);
    QVector<int> roles = { FileStatusRole, FileProgressRole };
    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::onDownLoadProgress(uint32_t id, uint8_t progress, bool is_end)
{

    auto target_file = findFileInfoById(id);
    target_file.second.file_status = is_end ? FileStatus::StatusDownloadCompleted : FileStatus::StatusDownloading;
    target_file.second.progress = static_cast<int>(progress);

    QModelIndex model_index = index(target_file.first, 0);
    QVector<int> roles = { FileStatusRole, FileProgressRole };

    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::cleanTmpFiles()
{
    QDir dir(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir));
    
    if (!dir.exists()) {
        return;
    }
    
    dir.removeRecursively();
}