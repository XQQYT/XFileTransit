# XFileTransit

[![Windows](https://img.shields.io/badge/platform-Windows-blue?logo=windows)](https://www.microsoft.com/windows) [![Linux](https://img.shields.io/badge/platform-Linux-green?logo=linux)](https://www.linux.org/)

XFileTransit 是一款基于 C++ 和 Qt 开发的跨平台局域网文件传输工具。支持文件拖拽，无需配置，局域网内高效安全共享文件。

[点此进入官网](https://xqqyt.top/) | [版本发布视频](https://www.bilibili.com/video/BV13R2XBJEgG/?share_source=copy_web)

[环境搭建视频](https://b23.tv/YJRrY3s) | [开发指南](https://github.com/XQQYT/XFileTransit/wiki/%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97) | [使用指南，必看！！](https://github.com/XQQYT/XFileTransit/wiki/%E4%BD%BF%E7%94%A8%E6%8C%87%E5%8D%97)

## 功能特点

- 文件拖拽，一键发送
- 局域网自动发现设备，无需配置服务器
- TLS+AES 加密传输，保障文件安全
- 支持文件列表去重、批量发送与接收
- 任务多线程处理，超高性能
- 简洁美观的 QML 界面

## 安装与运行

1. 环境依赖：**Qt 6.8.3**，**OpenSSL**，**CMake**， **C++17**
2. 下载源码：  
   ```bash
   git clone https://github.com/XQQYT/XFileTransit.git
   cd XFileTransit
   ```
### 平台特定配置

3. ##### Linux
   ```
   # 编辑 buildLinux.sh，将 Qt6_PATH 设置为你的 Qt 安装路径
   # 运行构建脚本
   bash buildLinux.sh
   ```
4. ##### Windows
   ```
   编辑 mingw-toolchain.cmake，参考源码填写路径
   编辑 src/source/driver/CMakeLists.txt，设置 OPENSSL_ROOT_DIR 为你的 OpenSSL 路径
   运行构建脚本：
   buildWindow.bat
   ```
5. 输出：
   ```
   Debug版本带控制台输出
   Release版本没有控制台输出
   可执行程序位于build-xxx-xxx/src下
   ```
   
## 使用说明

1. 在应用窗口中拖拽要共享的文件；
2. 其他同一局域网内的用户可在窗口直接拖出或选择文件进行下载；
3. 支持任意文件格式，自动显示文件名称与图标。

## 参与开发

欢迎 PR 和 issue！

## 许可证

MIT License

## 联系方式

- 项目主页： [https://github.com/XQQYT/XFileTransit](https://github.com/XQQYT/XFileTransit)
- 开发者邮箱：xqqyt0502@163.com
