#include "model/FileListModel.h"
#include "model/ModelManager.h"
#include "control/EventBusManager.h"
#include "control/GlobalStatusManager.h"
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>

FileListModel::FileListModel(QObject *parent) : QAbstractListModel(parent)
{
    // FileInfo默认为LOW
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);

    EventBusManager::instance().subscribe("/sync/have_expired_file",
                                          std::bind(&FileListModel::onHaveExpiredFile,
                                                    this,
                                                    std::placeholders::_1));
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
                                                    std::placeholders::_3,
                                                    std::placeholders::_4));
    EventBusManager::instance().subscribe("/file/download_progress",
                                          std::bind(&FileListModel::onDownLoadProgress,
                                                    this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2,
                                                    std::placeholders::_3,
                                                    std::placeholders::_4));
    EventBusManager::instance().subscribe("/file/have_cancel_transit",
                                          std::bind(&FileListModel::onHaveCancelFile,
                                                    this,
                                                    std::placeholders::_1));
}

FileListModel::~FileListModel()
{
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : file_list.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= file_list.size())
        return QVariant();

    const FileInfo &file = file_list.at(index.row());

    switch (role)
    {
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
    case FileSpeedRole:
        return file.speed;
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
        {FileSpeedRole, "fileSpeed"},
        {Qt::ToolTipRole, "toolTip"}};
    return roles;
}

std::pair<int, FileInfo &> FileListModel::findFileInfoById(uint32_t id)
{
    auto it = std::find_if(file_list.begin(), file_list.end(),
                           [id](const FileInfo &info)
                           {
                               return info.id == id;
                           });
    if (it == file_list.end())
    {
        throw std::runtime_error("FileInfo with id " + std::to_string(id) + " not found");
    }
    int index = std::distance(file_list.begin(), it);
    return std::pair<int, FileInfo &>(index, *it);
}

// 添加本地文件
void FileListModel::addFiles(const QList<QString> &files, bool is_remote_file)
{
    if (files.isEmpty())
        return;

    QVector<FileInfo> unique_files;
    std::vector<std::string> files_to_send;
    for (const QString &file : files)
    {
        if (!isFileExists(file))
        {
            FileInfo cur_file(is_remote_file, file);
            // 步长为3
            if (GlobalStatusManager::getInstance().getConnectStatus())
            {
                files_to_send.push_back(std::to_string(cur_file.id));
                files_to_send.push_back(std::to_string(cur_file.is_folder));
                files_to_send.push_back(cur_file.file_name.toUtf8().constData());
                files_to_send.push_back(std::to_string(cur_file.file_size));
            }
            unique_files.append(std::move(cur_file));
        }
    }

    // 只有存在新文件时才插入
    if (!unique_files.isEmpty())
    {
        beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + unique_files.size() - 1);
        file_list.append(unique_files);
        endInsertRows();
        if (auto_expand)
        {
            emit mainWinExpand();
        }
    }

    if (GlobalStatusManager::getInstance().getConnectStatus() && !files_to_send.empty())
    {
        EventBusManager::instance().publish("/sync/send_addfiles", files_to_send, uint8_t(4));
    }
}

void FileListModel::addRemoteFiles(std::vector<std::vector<std::string>> files)
{
    QVector<FileInfo> remote_files;
    for (const auto &file : files)
    {
        remote_files.append(FileInfo(true,
                                     std::stoul(file[0]), std::stoi(file[1]) != 0,
                                     QString::fromStdString(file[2]), std::stoull(file[3])));
        GlobalStatusManager::getInstance().insertFile(remote_files.back().id, remote_files.back().file_name.toUtf8().constData());
    }

    if (!remote_files.empty())
    {
        beginInsertRows(QModelIndex(), file_list.size(), file_list.size() + remote_files.size() - 1);
        file_list.append(remote_files);
        endInsertRows();
        if (auto_expand)
        {
            emit mainWinExpand();
        }
    }
    if (auto_download)
    {
        for (int i = 0; i < file_list.size(); ++i)
        {
            if (file_list[i].file_size <= auto_download_file_size && file_list[i].file_status == StatusRemoteDefault)
            {
                downloadFile(i);
            }
        }
    }
}

bool FileListModel::isFileExists(const QString &filePath)
{
    return std::any_of(file_list.begin(), file_list.end(),
                       [&](const FileInfo &info)
                       {
                           return info.file_url == filePath;
                       });
}

void FileListModel::removeFile(int index)
{
    if (index < 0 || index >= file_list.size())
        return;

    if (GlobalStatusManager::getInstance().getConnectStatus())
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
    for (auto it = file_list.begin(); it < file_list.end(); it++)
    {
        if (it->is_remote_file)
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
    for (auto &file : file_list)
    {
        file.id = GlobalStatusManager::getInstance().getFileId();
    }
}

void FileListModel::syncCurrentFiles()
{
    std::vector<std::string> files_to_send;
    files_to_send.reserve(file_list.size());
    for (auto &cur_file : file_list)
    {
        if (cur_file.is_remote_file)
            continue;
        // 先更新当前所有文件的id
        cur_file.id = GlobalStatusManager::getInstance().getFileId();

        files_to_send.push_back(std::to_string(cur_file.id));
        files_to_send.push_back(std::to_string(cur_file.is_folder));
        files_to_send.push_back(cur_file.file_name.toUtf8().constData());
        files_to_send.push_back(std::to_string(cur_file.file_size));
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
    if (file_list[index].file_status == FileStatus::StatusPending)
    {
        EventBusManager::instance().publish("/file/cancel_file_send", static_cast<uint32_t>(file_id));
    }
}

void FileListModel::onHaveExpiredFile(std::vector<std::string> id)
{
    for (auto &file_id : id)
    {
        uint32_t target_id = std::stoul(file_id);
        auto file = findFileInfoById(target_id);
        file.second.file_status = FileStatus::StatusError;
        QModelIndex model_index = index(file.first, 0);
        QVector<int> roles = {FileStatusRole};

        emit dataChanged(model_index, model_index, roles);
    }
}

void FileListModel::removeFileById(std::vector<std::string> id)
{
    if (id.empty())
        return;
    std::vector<int> indicesToRemove;
    for (const auto &fileIdStr : id)
    {
        uint32_t target_id = std::stoul(fileIdStr);
        for (int i = 0; i < file_list.size(); ++i)
        {
            if (file_list[i].id == target_id)
            {
                indicesToRemove.push_back(i);
                GlobalStatusManager::getInstance().removeFile(target_id);
                if (file_list[i].file_status == FileStatus::StatusPending)
                {
                    EventBusManager::instance().publish("/file/cancel_file_send", static_cast<uint32_t>(file_list[i].id));
                }
                break;
            }
        }
    }
    if (indicesToRemove.empty())
        return;
    std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
    for (int index : indicesToRemove)
    {
        if (index >= 0 && index < file_list.size())
        {
            beginRemoveRows(QModelIndex(), index, index);
            file_list.removeAt(index);
            endRemoveRows();
        }
    }
    if (auto_expand)
    {
        emit mainWinExpand();
    }
}
void FileListModel::downloadFile(int i)
{
    EventBusManager::instance().publish("/file/send_get_file", uint32_t(file_list[i].id));

    file_list[i].file_status = FileStatus::StatusPending;

    QModelIndex model_index = index(i, 0);
    QVector<int> roles = {FileStatusRole};

    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::haveDownLoadRequest(std::vector<std::string> file_ids)
{
    for (auto id : file_ids)
    {
        uint32_t target_id = std::stoul(id);
        auto target_file = findFileInfoById(target_id);
        // 文件失效
        if (!FileSystemUtils::fileIsExist(target_file.second.source_path.toStdString()))
        {
            target_file.second.file_status = FileStatus::StatusError;
            EventBusManager::instance().publish("/sync/send_expired_file", target_id);
        }
        else
        {
            target_file.second.file_status = FileStatus::StatusPending;
            EventBusManager::instance().publish("/file/have_file_to_send", target_id, target_file.second.source_path.toStdString());

            target_file.second.file_status = FileStatus::StatusPending;
        }

        QModelIndex model_index = index(target_file.first, 0);
        QVector<int> roles = {FileStatusRole};

        emit dataChanged(model_index, model_index, roles);
    }
}

void FileListModel::onUploadFileProgress(uint32_t id, uint8_t progress, uint32_t speed, bool is_end)
{
    auto target_file = findFileInfoById(id);

    if (target_file.second.file_status == FileStatus::StatusUploadCancel)
        return;

    // 更新状态和进度
    target_file.second.file_status = is_end ? FileStatus::StatusUploadCompleted : FileStatus::StatusUploading;
    target_file.second.progress = static_cast<quint8>(progress);
    if (auto_expand && is_end)
    {
        emit mainWinExpand();
    }

    // 使用移动平均计算速度
    if (!is_end)
    {
        if (!speed_history.contains(id))
        {
            speed_history[id] = QVector<uint32_t>();
        }
        auto &history = speed_history[id];
        history.append(speed);
        // 保持30个记录
        const int MAX_HISTORY_SIZE = 30;
        if (history.size() > MAX_HISTORY_SIZE)
        {
            history.removeFirst();
        }

        // 计算平均值
        uint64_t sum = 0;
        for (auto s : history)
        {
            sum += s;
        }
        target_file.second.speed = static_cast<quint32>(sum / history.size());
    }
    else
    {
        target_file.second.speed = 0;
        speed_history.remove(id);
    }

    QModelIndex model_index = index(target_file.first, 0);
    QVector<int> roles = {FileStatusRole, FileProgressRole, FileSpeedRole};
    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::onDownLoadProgress(uint32_t id, uint8_t progress, uint32_t speed, bool is_end)
{
    auto target_file = findFileInfoById(id);

    if (target_file.second.file_status == FileStatus::StatusRemoteDefault)
        return;

    target_file.second.file_status = is_end ? FileStatus::StatusDownloadCompleted : FileStatus::StatusDownloading;
    target_file.second.progress = static_cast<int>(progress);
    if (auto_expand && is_end)
    {
        emit mainWinExpand();
    }
    // 使用移动平均计算速度
    if (!is_end)
    {
        if (!speed_history.contains(id))
        {
            speed_history[id] = QVector<uint32_t>();
        }
        auto &history = speed_history[id];
        history.append(speed);
        // 保持30个记录
        const int MAX_HISTORY_SIZE = 30;
        if (history.size() > MAX_HISTORY_SIZE)
        {
            history.removeFirst();
        }

        // 计算平均值
        uint64_t sum = 0;
        for (auto s : history)
        {
            sum += s;
        }
        target_file.second.speed = static_cast<quint32>(sum / history.size());
    }
    else
    {
        target_file.second.speed = 0;
        speed_history.remove(id);
    }

    QModelIndex model_index = index(target_file.first, 0);
    QVector<int> roles = {FileStatusRole, FileProgressRole};

    emit dataChanged(model_index, model_index, roles);
}

void FileListModel::onHaveCancelFile(uint32_t id)
{
    for (int i = 0; i < file_list.size(); ++i)
    {
        if (file_list[i].id == id)
        {
            file_list[i].file_status = FileStatus::StatusDownloadCancel;
            QModelIndex model_index = index(i, 0);
            QVector<int> roles = {FileStatusRole};
            emit dataChanged(model_index, model_index, roles);
        }
    }
}

void FileListModel::cleanTmpFiles()
{
    if (!auto_clear_cache)
    {
        return;
    }
    QDir dir(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir));

    if (!dir.exists())
    {
        return;
    }

    dir.removeRecursively();
    QDir().mkpath(QString::fromStdString(GlobalStatusManager::absolute_tmp_dir));
}

bool FileListModel::isTransferring()
{
    for (auto &file : file_list)
    {
        if (file.file_status == FileStatus::StatusDownloading || file.file_status == FileStatus::StatusUploading)
        {
            return true;
        }
    }
    return false;
}

void FileListModel::updateFilePath(QString new_path)
{
    for (auto &i : file_list)
    {
        if (i.is_remote_file)
        {
            i.source_path = new_path + i.file_name;
            i.file_url = QUrl::fromLocalFile(i.source_path);
        }
    }
}

void FileListModel::setAutoDownload(bool enable)
{
    auto_download = enable;
}

void FileListModel::onSettingsChanged(Settings::Item item, QVariant value)
{
    switch (item)
    {
    case Settings::Item::Theme:
        emit themeChanged(value.toInt());
        break;
    case Settings::Item::CachePath:
        updateFilePath(value.toString());
        break;
    case Settings::Item::AutoDownload:
        setAutoDownload(value.toBool());
    case Settings::Item::ExpandOnAction:
        auto_expand = value.toBool();
        break;
    case Settings::Item::AutoClearCache:
        auto_clear_cache = value.toBool();
        break;
    default:
        return;
    }
}

void FileListModel::cancelTransit(int i)
{
    FileInfo &info = file_list[i];
    qDebug() << "cancel transit id: " << info.id;

    if (info.file_status == FileStatus::StatusUploading)
    {
        EventBusManager::instance().publish("/file/cancel_transit_in_sender", static_cast<uint32_t>(info.id));

        info.file_status = FileStatus::StatusUploadCancel;
        QModelIndex model_index = index(i, 0);
        QVector<int> roles = {FileStatusRole};
        emit dataChanged(model_index, model_index, roles);
    }
    else if (info.file_status == FileStatus::StatusDownloading)
    {
        EventBusManager::instance().publish("/file/cancel_transit_in_receiver", static_cast<uint32_t>(info.id));
    }
    else
    {
        LOG_ERROR("Invalid file status: " << static_cast<int>(info.file_status));
    }
}
