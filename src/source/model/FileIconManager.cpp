#include "model/FileIconManager.h"
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

FileIconManager &FileIconManager::getInstance()
{
    static FileIconManager instance;
    return instance;
}

FileIconManager::FileIconManager()
    : icon_provider(std::make_unique<QFileIconProvider>())
{
    // 创建临时图标目录
    temp_icon_dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/file_icons/";
    QDir().mkpath(temp_icon_dir);
}

FileIconManager::~FileIconManager()
{
    clearCache();
}

QUrl FileIconManager::getFileIcon(const QString &file_url, bool is_folder)
{
    if (is_folder)
    {
        return QUrl("qrc:/file_icon/FileIcons/folder.svg");
    }

    // 生成缓存键
    QFileInfo file_info(file_url);
    QString cache_key = file_info.suffix().toLower();
    if (cache_key.isEmpty())
    {
        cache_key = "file";
    }

    // 检查缓存
    if (icon_cache.contains(cache_key))
    {
        return icon_cache[cache_key];
    }

    // 获取系统图标
    QString icon_path = getFileTypeIcon(file_url);

    QUrl icon_url;
    if (!icon_path.isEmpty())
    {
        icon_url = QUrl::fromLocalFile(icon_path);
        icon_cache[cache_key] = icon_url;
    }
    else
    {
        // 如果获取系统图标失败，使用默认图标
        icon_cache[cache_key] = QUrl("qrc:/file_icon/FileIcons/unknow_file.svg");
    }

    return icon_url;
}

QString FileIconManager::getFileTypeIcon(const QString &file_url)
{
    QFileInfo file_info(file_url);
    QIcon icon;

    if (file_info.exists())
    {
        icon = icon_provider->icon(file_info);
    }
    else
    {
        // 如果文件不存在，使用文件类型图标
        icon = icon_provider->icon(QFileIconProvider::File);
    }

    if (icon.isNull())
    {
        return QString();
    }

    QString suffix = file_info.suffix().toLower();
    if (suffix.isEmpty())
    {
        suffix = "file";
    }

    return saveIconToTemp(icon, suffix);
}

QUrl FileIconManager::getFileIconBySuffix(const QString &suffix, bool is_folder)
{
    if (is_folder)
    {
        return QUrl("qrc:/file_icon/FileIcons/folder.svg");
    }

    // 生成缓存键
    QString cache_key = suffix.toLower();
    if (cache_key.isEmpty())
    {
        cache_key = "file";
    }

    // 检查缓存
    if (icon_cache.contains(cache_key))
    {
        return icon_cache[cache_key];
    }

    // 获取系统图标
    QString icon_path = getFileTypeIconBySuffix(suffix);

    QUrl icon_url;
    if (!icon_path.isEmpty())
    {
        icon_url = QUrl::fromLocalFile(icon_path);
        icon_cache[cache_key] = icon_url;
    }
    else
    {
        // 如果获取系统图标失败，使用默认图标
        icon_cache[cache_key] = QUrl("qrc:/file_icon/FileIcons/unknow_file.svg");
    }

    return icon_url;
}

QString FileIconManager::getFileTypeIconBySuffix(const QString &suffix)
{
    QIcon icon;

    // 获取特定后缀的图标
    QString temp_file_path;
    if (suffix.isEmpty())
    {
        temp_file_path = temp_icon_dir + "dummy_file";
    }
    else
    {
        temp_file_path = temp_icon_dir + "dummy." + suffix;
    }

    // 创建临时文件以获取图标
    QFile temp_file(temp_file_path);
    if (!temp_file.exists())
    {
        if (temp_file.open(QIODevice::WriteOnly))
        {
            temp_file.close();
        }
    }

    QFileInfo temp_file_info(temp_file_path);
    icon = icon_provider->icon(temp_file_info);

    // 清理临时文件
    temp_file.remove();

    if (icon.isNull())
    {
        // 如果获取失败，使用通用文件图标
        icon = icon_provider->icon(QFileIconProvider::File);
    }

    QString cache_suffix = suffix.toLower();
    if (cache_suffix.isEmpty())
    {
        cache_suffix = "file";
    }

    return saveIconToTemp(icon, cache_suffix);
}

QString FileIconManager::saveIconToTemp(const QIcon &icon, const QString &type)
{
    if (icon.isNull())
    {
        return QString();
    }

    // 生成临时文件路径
    QString temp_file = temp_icon_dir + type + ".png";

    // 如果文件已存在且未过期，直接返回
    if (QFile::exists(temp_file))
    {
        QFileInfo file_info(temp_file);
        if (file_info.lastModified().secsTo(QDateTime::currentDateTime()) < 86400)
        {
            return temp_file;
        }
    }

    // 获取合适大小的图标
    int icon_size = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    if (icon_size <= 0)
    {
        icon_size = 32; // 默认大小
    }

    QPixmap pixmap = icon.pixmap(icon_size, icon_size);
    if (pixmap.save(temp_file, "PNG"))
    {
        return temp_file;
    }

    return QString();
}

void FileIconManager::clearCache()
{
    // 清理临时文件
    QDir dir(temp_icon_dir);
    if (dir.exists())
    {
        dir.removeRecursively();
    }
    QDir().mkpath(temp_icon_dir);
    icon_cache.clear();
}

void FileIconManager::preloadCommonIcons()
{
    // 预加载常用图标
    QStringList common_files = {
        "test.txt", "test.doc", "test.docx", "test.pdf",
        "test.jpg", "test.png", "test.gif", "test.bmp",
        "test.mp3", "test.mp4", "test.avi", "test.zip",
        "test.rar", "test.exe", "test.html", "test.cpp"};

    for (const QString &file : common_files)
    {
        getFileIcon(file);
    }

    // 也预加载文件夹图标
    getFileIcon("/dummy/path", true);
}