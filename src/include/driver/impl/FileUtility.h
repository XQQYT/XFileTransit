#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

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
                    leafFiles.push_back(entry.path().string());
                }
            }
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "遍历目录错误: " << ex.what() << std::endl;
        }

        return leafFiles;
    }

    static std::vector<std::string> findAllLeafFolders(const std::string& rootPath) {
        std::vector<std::string> leafFolders;

        if (!fs::exists(rootPath)) {
            std::cerr << "路径不存在: " << rootPath << std::endl;
            return leafFolders;
        }

        // 如果是文件，返回空（不处理文件）
        if (fs::is_regular_file(rootPath)) {
            return leafFolders;
        }

        // 递归查找叶子文件夹
        findLeafFoldersRecursive(rootPath, leafFolders);
        return leafFolders;
    }

private:
    static void findLeafFoldersRecursive(const std::string& currentPath,
        std::vector<std::string>& leafFolders) {
        try {
            bool hasSubdirectories = false;

            // 遍历当前目录
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (entry.is_directory()) {
                    hasSubdirectories = true;
                    findLeafFoldersRecursive(entry.path().string(), leafFolders);
                }
            }

            // 如果没有子目录，当前目录就是叶子文件夹
            if (!hasSubdirectories) {
                leafFolders.push_back(currentPath);
            }

        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "访问路径错误: " << currentPath << " - " << ex.what() << std::endl;
        }
    }
};


#endif