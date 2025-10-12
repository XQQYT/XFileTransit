#ifndef _FILEICONMANAGER_H
#define _FILEICONMANAGER_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QSet>

class FileIconManager
{
public:
    static FileIconManager& getInstance();
    QString getFileIconSvg(const QString& fu, bool irf = false);

private:
    FileIconManager();
    void initDefaultIcons();
    void initImagePredix();
    QHash<QString, QString> default_icons;
    QSet<QString> img_prefix;
};

#endif // FILEICONMANAGER_H