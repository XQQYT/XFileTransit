#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <climits>

// 平台特定的头文件
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h> // 这个头文件用于 readlink
#include <limits.h> // 这个头文件用于 PATH_MAX
#endif

namespace fs = std::filesystem;

class FileSystemUtils
{
public:
    // 获取文件大小（字节）
    static uint64_t getFileSize(const std::string &filePath)
    {
        try
        {
            if (!fs::exists(filePath))
            {
                std::cerr << "文件不存在: " << filePath << std::endl;
                return 0;
            }

            if (fs::is_directory(filePath))
            {
                std::cerr << "路径是目录，不是文件: " << filePath << std::endl;
                return 0;
            }

            return fs::file_size(filePath);
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "获取文件大小错误: " << ex.what() << std::endl;
            return 0;
        }
    }

    static bool fileIsExist(std::string file_path)
    {
        return fs::exists(file_path);
    }

    // 判断是否为文件夹
    static bool isDirectory(const std::string &path)
    {
        try
        {
            return fs::exists(path) && fs::is_directory(path);
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "判断目录错误: " << ex.what() << std::endl;
            return false;
        }
    }

    static std::vector<std::string> findAllLeafFolders(const std::string &rootPath, uint32_t &total_paths)
    {
        std::vector<std::string> leafFolders;

        if (!fs::exists(rootPath))
        {
            std::cerr << "路径不存在: " << rootPath << std::endl;
            return leafFolders;
        }

        if (fs::is_regular_file(rootPath))
        {
            return leafFolders;
        }

        fs::path basePath = fs::absolute(rootPath).lexically_normal();

        findLeafFoldersRecursive(basePath, basePath, leafFolders);
        total_paths = static_cast<uint32_t>(leafFolders.size());
        return leafFolders;
    }

    static std::string vectorToJsonString(const std::vector<std::string> &paths)
    {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < paths.size(); ++i)
        {
            if (i > 0)
            {
                oss << ",";
            }
            std::string escapedPath;
            for (char c : paths[i])
            {
                if (c == '\\')
                {
                    escapedPath += "\\\\";
                }
                else if (c == '\"')
                {
                    escapedPath += "\\\"";
                }
                else if (c == '\n')
                {
                    escapedPath += "\\n";
                }
                else if (c == '\t')
                {
                    escapedPath += "\\t";
                }
                else if (c == '\r')
                {
                    escapedPath += "\\r";
                }
                else
                {
                    escapedPath += c;
                }
            }

            oss << "\"" << escapedPath << "\"";
        }
        oss << "]";
        return oss.str();
    }

    static std::string absoluteToRelativePath(const std::string &absolutePath, const std::string &referenceDir)
    {
        try
        {
            fs::path absPath(absolutePath);
            fs::path refDir(referenceDir);

            // 确保参考路径是绝对路径
            if (!refDir.is_absolute())
            {
                refDir = fs::absolute(refDir);
            }

            // 使用 filesystem 的相对路径函数
            fs::path relativePath = fs::relative(absPath, refDir);

            return relativePath.string();
        }
        catch (const std::exception &e)
        {
            std::cerr << "相对路径转换错误: " << e.what() << std::endl;
            // 如果出错，返回原始路径
            return absolutePath;
        }
    }

    static bool createDirectoryRecursive(const std::string &path)
    {
        try
        {
            std::error_code ec;
            bool result = fs::create_directories(path, ec);
            if (ec)
            {
                std::cerr << "创建目录错误: " << ec.message() << std::endl;
                return false;
            }
            return result;
        }
        catch (const std::exception &e)
        {
            std::cerr << "创建目录异常: " << e.what() << std::endl;
            return false;
        }
    }

    // 宽字符版本的重载
    static bool createDirectoryRecursive(const std::wstring &path)
    {
        try
        {
            std::error_code ec;
            bool result = fs::create_directories(path, ec);
            if (ec)
            {
                std::cerr << "创建目录错误: " << ec.message() << std::endl;
                return false;
            }
            return result;
        }
        catch (const std::exception &e)
        {
            std::cerr << "创建目录异常: " << e.what() << std::endl;
            return false;
        }
    }

    static bool directoryExists(const std::string &path)
    {
        std::error_code ec;
        return fs::exists(path, ec) && fs::is_directory(path, ec);
    }

    // 宽字符版本的重载
    static bool directoryExists(const std::wstring &path)
    {
        std::error_code ec;
        return fs::exists(path, ec) && fs::is_directory(path, ec);
    }

    // UTF-8 转宽字符
    static std::wstring utf8ToWide(const std::string &utf8_str)
    {
        if (utf8_str.empty())
            return L"";

#ifdef _WIN32
// Windows 实现
#include <windows.h>
        int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
        if (wide_len == 0)
            return L"";

        std::wstring wide_str;
        wide_str.resize(wide_len - 1); // 减去 null 终止符

        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], wide_len);
        return wide_str;
#else
        std::wstring wide_str;
        wide_str.reserve(utf8_str.size());

        for (size_t i = 0; i < utf8_str.size();)
        {
            wchar_t wc = L'\0';
            size_t bytes = 1;

            // UTF-8 解码
            unsigned char c = static_cast<unsigned char>(utf8_str[i]);
            if (c < 0x80)
            {
                wc = c;
            }
            else if (c < 0xE0)
            {
                if (i + 1 < utf8_str.size())
                {
                    wc = ((c & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
                    bytes = 2;
                }
            }
            else if (c < 0xF0)
            {
                if (i + 2 < utf8_str.size())
                {
                    wc = ((c & 0x0F) << 12) | ((utf8_str[i + 1] & 0x3F) << 6) | (utf8_str[i + 2] & 0x3F);
                    bytes = 3;
                }
            }
            else
            {
                if (i + 3 < utf8_str.size())
                {
                    wc = L'?';
                    bytes = 4;
                }
            }

            wide_str.push_back(wc);
            i += bytes;
        }

        return wide_str;
#endif
    }

    static uint64_t calculateFolderSize(const fs::path &path)
    {
        try
        {
            if (!fs::exists(path))
            {
                std::cerr << "路径不存在: " << path.string() << std::endl;
                return 0;
            }
            if (fs::is_regular_file(path))
            {
                return fs::file_size(path);
            }
            uint64_t totalSize = 0;
            std::error_code ec;
            for (const auto &entry : fs::recursive_directory_iterator(path,
                                                                      fs::directory_options::skip_permission_denied, ec))
            {
                if (ec)
                {
                    std::cerr << "访问目录项出错: " << ec.message() << std::endl;
                    continue;
                }
                if (fs::is_regular_file(entry.status()))
                {
                    uint64_t size = fs::file_size(entry.path(), ec);
                    if (!ec)
                    {
                        totalSize += size;
                    }
                    else
                    {
                        std::cerr << "获取文件大小失败: " << entry.path().string()
                                  << " - " << ec.message() << std::endl;
                    }
                }
            }
            return totalSize;
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "文件系统错误: " << e.what()
                      << " - 错误码: " << e.code().value() << std::endl;
            return static_cast<uint64_t>(-1);
        }
        catch (const std::exception &e)
        {
            std::cerr << "未知错误: " << e.what() << std::endl;
            return static_cast<uint64_t>(-1);
        }
    }

    static std::string getExecutableDirectory()
    {
#ifdef _WIN32
#include <windows.h>
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (length == 0 || length >= MAX_PATH)
        {
            return "";
        }

        fs::path exePath(buffer);
        fs::path exeDir = exePath.parent_path();
#else
        char buffer[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
        if (count == -1)
        {
            return "";
        }
        buffer[count] = '\0';

        fs::path exePath(buffer);
        fs::path exeDir = exePath.parent_path();
#endif

        std::string dir = exeDir.u8string();

        // 将反斜杠转换为正斜杠
        std::replace(dir.begin(), dir.end(), '\\', '/');

        // 确保以斜杠结尾
        if (!dir.empty() && dir.back() != '/')
        {
            dir += '/';
        }

        return dir;
    }

    // 深度优先搜索获取所有叶子文件路径
    static std::vector<std::string> findAllLeafFiles(const std::string &rootPath)
    {
        std::vector<std::string> leafFiles;

        if (!fs::exists(rootPath))
        {
            std::cerr << "根路径不存在: " << rootPath << std::endl;
            return leafFiles;
        }

        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(
                     rootPath, fs::directory_options::skip_permission_denied))
            {

                if (entry.is_regular_file())
                {
                    fs::path filePath = entry.path();
                    filePath = filePath.make_preferred();
                    leafFiles.push_back(filePath.string());
                }
            }
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "遍历目录错误: " << ex.what() << std::endl;
        }

        return leafFiles;
    }

    // 获取目录中的所有文件（非递归）
    static std::vector<std::string> getFilesInDirectory(const std::string &dirPath)
    {
        std::vector<std::string> files;

        if (!fs::exists(dirPath))
        {
            std::cerr << "目录不存在: " << dirPath << std::endl;
            return files;
        }

        try
        {
            for (const auto &entry : fs::directory_iterator(dirPath))
            {
                if (entry.is_regular_file())
                {
                    files.push_back(entry.path().string());
                }
            }
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "读取目录错误: " << ex.what() << std::endl;
        }

        return files;
    }

    // 获取目录中的所有子目录（非递归）
    static std::vector<std::string> getDirectoriesInDirectory(const std::string &dirPath)
    {
        std::vector<std::string> dirs;

        if (!fs::exists(dirPath))
        {
            std::cerr << "目录不存在: " << dirPath << std::endl;
            return dirs;
        }

        try
        {
            for (const auto &entry : fs::directory_iterator(dirPath))
            {
                if (entry.is_directory())
                {
                    dirs.push_back(entry.path().string());
                }
            }
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "读取目录错误: " << ex.what() << std::endl;
        }

        return dirs;
    }

    // 复制文件
    static bool copyFile(const std::string &source, const std::string &destination, bool overwrite = true)
    {
        try
        {
            std::error_code ec;
            if (overwrite)
            {
                fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
            }
            else
            {
                fs::copy_file(source, destination, ec);
            }

            if (ec)
            {
                std::cerr << "复制文件错误: " << ec.message() << std::endl;
                return false;
            }
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "复制文件异常: " << e.what() << std::endl;
            return false;
        }
    }

    // 删除文件或目录
    static bool removeFileOrDirectory(const std::string &path)
    {
        try
        {
            std::error_code ec;
            uintmax_t count = fs::remove_all(path, ec);

            if (ec)
            {
                std::cerr << "删除错误: " << ec.message() << std::endl;
                return false;
            }

            return count > 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "删除异常: " << e.what() << std::endl;
            return false;
        }
    }

private:
    static void findLeafFoldersRecursive(const fs::path &basePath,
                                         const fs::path &currentPath,
                                         std::vector<std::string> &leafFolders)
    {
        try
        {
            bool hasSubdirectories = false;

            for (const auto &entry : fs::directory_iterator(currentPath))
            {
                if (entry.is_directory())
                {
                    hasSubdirectories = true;
                    findLeafFoldersRecursive(basePath, entry.path(), leafFolders);
                }
            }

            if (!hasSubdirectories)
            {
                fs::path relativePath = fs::relative(currentPath, basePath);
                fs::path preferredPath = relativePath.make_preferred();
                leafFolders.push_back(preferredPath.string());
            }
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "访问路径错误: " << currentPath.string() << " - " << ex.what() << std::endl;
        }
    }
};

#endif // FILEUTILITY_H