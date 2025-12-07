#ifndef FILEICONMANAGER_H
#define FILEICONMANAGER_H

#include <QtCore/QUrl>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtWidgets/QFileIconProvider>
#include <memory>

class FileIconManager
{
public:
    static FileIconManager &getInstance();

    QUrl getFileIcon(const QString &file_url, bool is_folder = false);
    void clearCache();
    void preloadCommonIcons();
    QUrl getFileIconBySuffix(const QString &suffix, bool is_folder = false);

private:
    FileIconManager();
    ~FileIconManager();

    QString getFileTypeIcon(const QString &file_url);
    QString getFileTypeIconBySuffix(const QString &suffix);
    QString saveIconToTemp(const QIcon &icon, const QString &type);

private:
    QHash<QString, QUrl> icon_cache;
    std::unique_ptr<QFileIconProvider> icon_provider;
    QString temp_icon_dir;
};

#endif // FILEICONMANAGER_H