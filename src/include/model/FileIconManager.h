#ifndef _FILEICONMANAGER_H
#define _FILEICONMANAGER_H

#include <QtCore/QUrl>
#include <QtCore/QHash>
#include <QtCore/QSet>

struct HICON__;
typedef struct HICON__* HICON;

class FileIconManager
{
public:
    static FileIconManager& getInstance();
    
    QUrl getFileIcon(const QString& file_url, bool isfolder = false);
    void clearCache();
    void preloadCommonIcons();

private:
    FileIconManager();
    ~FileIconManager();
    
    QString getFileTypeIcon(const QString& file_url);
    QString saveIconToTemp(HICON hIcon, const QString& type);
    
private:
    QHash<QString, QUrl> icon_cache;  // 图标缓存，存储 QUrl
};

#endif // _FILEICONMANAGER_H