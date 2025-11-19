# XFileTransit

XFileTransit 是一款基于 C++ 和 Qt 开发的 Windows 局域网文件传输工具。支持文件拖拽，无需配置，局域网内高效安全共享文件。

## 功能特点

- 文件拖拽，一键发送
- 局域网自动发现设备，无需配置服务器
- TLS+AES 加密传输，保障文件安全
- 支持文件列表去重、批量发送与接收
- 任务多线程处理，超高性能
- 简洁美观的 QML 界面

## 安装与运行

1. 环境依赖：**Qt 6.8.2**，**OpenSSL**，**CMake**
2. 下载源码并编译：  
   ```bash
   git clone https://github.com/XQQYT/XFileTransit.git
   cd XFileTransit
   mkdir build && cd build
   cmake ..
   make
   ```
3. 启动方式：运行 `XFileTransit.exe`，将文件拖入窗口即可共享。
   
## 使用说明

1. 在应用窗口中拖拽要共享的文件；
2. 其他同一局域网内的用户可在窗口直接拖出或选择文件进行下载；
3. 支持任意文件格式，自动显示文件名称与图标。

## 技术细节

- C++ 主体架构，QML 实现 UI
- 网络通信采用 TCP/TLS，自动生成临时证书
- 消息协议统一解析与组包，支持扩展
- 多线程任务队列，异步无阻塞

## 参与开发

欢迎 PR 和 issue！

## 许可证

MIT License

## 联系方式

- 项目主页： [https://github.com/XQQYT/XFileTransit](https://github.com/XQQYT/XFileTransit)
- 开发者邮箱：xqqyt0502@163.com
