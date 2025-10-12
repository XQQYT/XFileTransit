#include "model/FileIconManager.h"
#include <QtCore/QFileInfo>

FileIconManager& FileIconManager::getInstance()
{
    static FileIconManager instance;
    return instance;
}

FileIconManager::FileIconManager()
{
    initDefaultIcons();
    initImagePredix();
}

void FileIconManager::initDefaultIcons()
{
    default_icons["doc"] = "/file_icon/FileIcons/doc.svg"; default_icons["docx"] = "/file_icon/FileIcons/doc.svg";
    default_icons["xls"] = "/file_icon/FileIcons/xls.svg"; default_icons["xlsx"] = "/file_icon/FileIcons/xls.svg";
    default_icons["ppt"] = "/file_icon/FileIcons/ppt.svg"; default_icons["pptx"] = "/file_icon/FileIcons/ppt.svg";
    default_icons["mp3"] = "/file_icon/FileIcons/mp3.svg"; default_icons["mp4"] = "/file_icon/FileIcons/mp4.svg";
    default_icons["zip"] = "/file_icon/FileIcons/zip.svg"; default_icons["rar"] = "/file_icon/FileIcons/rar.svg";
    default_icons["exe"] = "/file_icon/FileIcons/exe.svg";
}
void FileIconManager::initImagePredix()
{
    img_prefix.insert("jpg");
    img_prefix.insert("png");
    img_prefix.insert("jpeg");
    img_prefix.insert("pdf");
}
QString FileIconManager::getFileIconSvg(const QString& fu, bool irf)
{
    quint64 index = fu.lastIndexOf('.') + 1;
    if (img_prefix.contains(fu.mid(index)))
    {
        return fu;
    }
    return default_icons[fu.mid(index)];
}
