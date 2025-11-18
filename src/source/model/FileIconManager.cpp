#include "model/FileIconManager.h"
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

#define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <shellapi.h>
    #include <shlobj.h>

FileIconManager& FileIconManager::getInstance()
{
    static FileIconManager instance;
    return instance;
}

FileIconManager::FileIconManager()
{
}

FileIconManager::~FileIconManager()
{
    clearCache();
}

QUrl FileIconManager::getFileIcon(const QString& file_url, bool is_folder)
{
    if(is_folder)
    {
        return QUrl("qrc:/file_icon/FileIcons/folder.svg");
    }
    // 生成缓存键
    QString cache_key = QFileInfo(file_url).suffix().toLower();
    if (cache_key.isEmpty()) {
        cache_key = "file";
    }
    
    // 检查缓存
    if (icon_cache.contains(cache_key)) {
        return icon_cache[cache_key];
    }
    
    // 获取系统图标
    QString icon_path = getFileTypeIcon(file_url);
    
    QUrl icon_url;
    if (!icon_path.isEmpty()) {
            icon_url = QUrl::fromLocalFile(icon_path);
            icon_cache[cache_key] = icon_url; 
    } else {
        // 如果获取系统图标失败，使用默认图标
        icon_cache[cache_key] = QUrl("qrc:/file_icon/FileIcons/unknow_file.svg");
    }
    
    return icon_url;
}

QString FileIconManager::getFileTypeIcon(const QString& file_url)
{
    SHFILEINFO shfi;
    std::wstring widePath = file_url.toStdWString();
    
    DWORD attributes = 0;
    if (!QFileInfo::exists(file_url)) {
        // 如果文件不存在，使用文件属性方式获取图标
        attributes = FILE_ATTRIBUTE_NORMAL;
    }
    
    if (SHGetFileInfo(widePath.c_str(), 
                     attributes, 
                     &shfi, 
                     sizeof(shfi), 
                     SHGFI_ICON | SHGFI_LARGEICON | (attributes ? SHGFI_USEFILEATTRIBUTES : 0))) {
        QString suffix = QFileInfo(file_url).suffix().toLower();
        if (suffix.isEmpty()) {
            suffix = "file";
        }
        return saveIconToTemp(shfi.hIcon, suffix);
    }
    
    return QString();
}

QString FileIconManager::saveIconToTemp(HICON hIcon, const QString& type)
{
    if (!hIcon) return QString();
    
    // 将 HICON 转换为 QIcon
    QIcon icon = QIcon(QPixmap::fromImage(QImage::fromHICON(hIcon)));
    DestroyIcon(hIcon);
    
    if (icon.isNull()) return QString();
    
    // 生成临时文件路径
    QString temp_dir = QDir::tempPath() + "/file_icons/";
    QDir().mkpath(temp_dir);
    
    QString temp_file = temp_dir + type + ".png";
    
    // 如果文件已存在且未过期，直接返回
    if (QFile::exists(temp_file)) {
        QFileInfo fileInfo(temp_file);
        if (fileInfo.lastModified().secsTo(QDateTime::currentDateTime()) < 3600) { // 1小时缓存
            return temp_file;
        }
    }
    
    // 保存图标到临时文件
    QPixmap pixmap = icon.pixmap(32, 32);
    if (pixmap.save(temp_file, "PNG")) {
        return temp_file;
    }
    
    return QString();
}

void FileIconManager::clearCache()
{
    // 清理临时文件
    QString tempDir = QDir::tempPath() + "/file_icons/";
    QDir dir(tempDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    icon_cache.clear();
}

void FileIconManager::preloadCommonIcons()
{
    // 预加载常用图标
    getFileIcon("test.txt");
    getFileIcon("test.doc");
    getFileIcon("test.pdf");
    getFileIcon("test.jpg");
    getFileIcon("test.png");
    getFileIcon("test.mp3");
    getFileIcon("test.mp4");
    getFileIcon("test.zip");
}