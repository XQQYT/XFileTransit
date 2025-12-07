// FileStreamHelper.h
#ifndef FILESTREAMHELPER_H
#define FILESTREAMHELPER_H

#include <fstream>
#include <memory>
#include <string>
#include <codecvt>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#else
#include <string>
#include <codecvt>
#include <locale>
#endif

namespace FileStreamHelper
{
    // 创建跨平台的输出文件流
    inline std::unique_ptr<std::ofstream> createOutputFileStream(const std::wstring &wpath)
    {
#ifdef _WIN32
        return std::make_unique<std::ofstream>(wpath.c_str(), std::ios::binary);
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string path = converter.to_bytes(wpath);
        return std::make_unique<std::ofstream>(path.c_str(), std::ios::binary);
#endif
    }

    // 创建跨平台的输入文件流
    inline std::unique_ptr<std::ifstream> createInputFileStream(const std::wstring &wpath)
    {
#ifdef _WIN32
        return std::make_unique<std::ifstream>(wpath.c_str(), std::ios::binary);
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string path = converter.to_bytes(wpath);
        return std::make_unique<std::ifstream>(path.c_str(), std::ios::binary);
#endif
    }

    // 宽字符串转本地路径字符串
    inline std::string wstringToLocalPath(const std::wstring &wstr)
    {
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
#endif
    }
}

#endif // FILESTREAMHELPER_H