#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h> 

namespace fs = std::filesystem;

class FileSystemUtils {
public:
    // 获取文件大小（字节）
    static uint64_t getFileSize(const std::string& filePath) {
        try {
            if (!fs::exists(filePath)) {
                std::cerr << "文件不存在: " << filePath << std::endl;
                return 0;
            }

            if (fs::is_directory(filePath)) {
                std::cerr << "路径是目录，不是文件: " << filePath << std::endl;
                return 0;
            }

            return fs::file_size(filePath);
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "获取文件大小错误: " << ex.what() << std::endl;
            return 0;
        }
    }

    // 判断是否为文件夹
    static bool isDirectory(const std::string& path) {
        try {
            return fs::exists(path) && fs::is_directory(path);
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "判断目录错误: " << ex.what() << std::endl;
            return false;
        }
    }

    static std::vector<std::string> findAllLeafFolders(const std::string& rootPath, uint32_t& total_paths) {
        std::vector<std::string> leafFolders;

        if (!fs::exists(rootPath)) {
            std::cerr << "路径不存在: " << rootPath << std::endl;
            return leafFolders;
        }

        if (fs::is_regular_file(rootPath)) {
            return leafFolders;
        }

        fs::path basePath = fs::absolute(rootPath).lexically_normal();

        findLeafFoldersRecursive(basePath, basePath, leafFolders);
        total_paths = leafFolders.size();
        return leafFolders;
    }

    static std::string vectorToJsonString(const std::vector<std::string>& paths) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i > 0) {
                oss << ",";
            }
            std::string escapedPath;
            for (char c : paths[i]) {
                if (c == '\\') {
                    escapedPath += "\\\\";
                }
                else if (c == '\"') {
                    escapedPath += "\\\"";
                }
                else {
                    escapedPath += c;
                }
            }

            oss << "\"" << escapedPath << "\"";
        }
        oss << "]";
        return oss.str();
    }

    static std::string absoluteToRelativePath(const std::string& absolutePath, const std::string& referenceDir) {
        namespace fs = std::filesystem;

        try {
            fs::path absPath(absolutePath);
            fs::path refDir(referenceDir);

            // 确保参考路径是绝对路径
            if (!refDir.is_absolute()) {
                refDir = fs::absolute(refDir);
            }

            // 使用 filesystem 的相对路径函数
            fs::path relativePath = fs::relative(absPath, refDir);

            return relativePath.string();
        }
        catch (const std::exception& e) {
            // 如果出错，回退到原始路径或抛出异常
            return absolutePath;
        }
    }
    static bool createDirectoryRecursive(const std::wstring& path) {
        std::error_code ec;
        bool result = std::filesystem::create_directories(path, ec);
        return result && !ec;
    }

    static bool directoryExists(const std::string& path) {
        std::error_code ec;
        return std::filesystem::exists(path, ec) &&
            std::filesystem::is_directory(path, ec);
    }
    static std::wstring utf8ToWide(const std::string& utf8_str) {
        if (utf8_str.empty()) return L"";

        int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
        if (wide_len == 0) return L"";

        std::wstring wide_str;
        wide_str.resize(wide_len - 1); // 减去 null 终止符

        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], wide_len);

        return wide_str;
    }
    static uint64_t calculateFolderSize(const fs::path& path) {
        try {
            if (!fs::exists(path)) {
                std::cerr << "路径不存在: " << path << std::endl;
                return 0;
            }
            if (fs::is_regular_file(path)) {
                return fs::file_size(path);
            }
            uint64_t totalSize = 0;
            std::error_code ec;
            for (const auto& entry : fs::recursive_directory_iterator(path, ec)) {
                if (ec) {
                    std::cerr << "访问目录项出错: " << ec.message() << std::endl;
                    continue;
                }
                if (fs::is_regular_file(entry.status())) {
                    uint64_t size = fs::file_size(entry.path(), ec);
                    if (!ec) {
                        totalSize += size;
                    }
                    else {
                        std::cerr << "获取文件大小失败: " << entry.path()
                            << " - " << ec.message() << std::endl;
                    }
                }
            }
            return totalSize;
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "文件系统错误: " << e.what()
                << " - 错误码: " << e.code() << std::endl;
            return static_cast<uint64_t>(-1);
        }
        catch (const std::exception& e) {
            std::cerr << "未知错误: " << e.what() << std::endl;
            return static_cast<uint64_t>(-1);
        }
    }

    static std::string getExecutableDirectory() {
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        fs::path exePath(buffer);
                fs::path exeDir = exePath.parent_path();
                std::string dir = exeDir.generic_string();
                if (!dir.empty() && dir.back() != '/') {
            dir += '/';
        }
        return dir;
    }
    // 深度优先搜索获取所有叶子文件路径
    static std::vector<std::string> findAllLeafFiles(const std::string& rootPath) {
        std::vector<std::string> leafFiles;

        if (!fs::exists(rootPath)) {
            std::cerr << "根路径不存在: " << rootPath << std::endl;
            return leafFiles;
        }

        try {
            for (const auto& entry : fs::recursive_directory_iterator(
                rootPath, fs::directory_options::skip_permission_denied)) {

                if (entry.is_regular_file()) {
                    // 创建副本，然后调用 make_preferred()
                    fs::path filePath = entry.path();  // 创建副本
                    filePath = filePath.make_preferred();  // 转换为首选分隔符
                    leafFiles.push_back(filePath.string());
                }
            }
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "遍历目录错误: " << ex.what() << std::endl;
        }

        return leafFiles;
    }
private:
    static void findLeafFoldersRecursive(const fs::path& basePath,
        const fs::path& currentPath,
        std::vector<std::string>& leafFolders) {
        try {
            bool hasSubdirectories = false;

            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (entry.is_directory()) {
                    hasSubdirectories = true;
                    findLeafFoldersRecursive(basePath, entry.path(), leafFolders);
                }
            }

            if (!hasSubdirectories) {
                fs::path relativePath = fs::relative(currentPath, basePath);
                fs::path preferredPath = relativePath.make_preferred();
                leafFolders.push_back(preferredPath.string());
            }
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "访问路径错误: " << currentPath << " - " << ex.what() << std::endl;
        }
    }
};


#endif