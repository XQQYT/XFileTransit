#ifndef FILESYNCENGINE_H
#define FILESYNCENGINE_H
/*
文件传输头部
{
  "type": "file_header",
  "content": {
    "id": "file_123456",
    "total_size": 10485760,
    "total_blocks": 100,
    "block_size": 104857
  }
}

文件夹结构头部
{
  "type": "dir_header",
  "content": {
    "id": "folder_789012",
    "leaf_paths": "[\"/project/src/main/java\",\"/project/src/test/java\"]",
    "total_paths": "4"
  }
}

目录项
{
  "type": "dir_item_header",
  "content": {
    "id": "file_123456",
    "path": "/project/src/main.java",
    "total_size": 10485760,
    "total_blocks": 100,
    "block_size": 104857
  }
}

文件块
struct FileBlock {
    uint32_t id;           // 与头部id对应
    uint32_t index;        // 块索引 (0-based)
    uint32_t data_size;    // 当前块实际数据大小
    uint8_t* data;        // 可变长度数据
};

文件接收流程:
accept连接-回调，有新连接-分配消息处理器上下文-接收消息-解析消息到结构体-回调-获取消息处理器上下文-处理*/

#include <stdint.h>

class FileSyncEngineInterface
{
public:
  inline static const uint32_t file_block_size = 128 * 1024;

public:
  struct FileBlock
  {
    uint32_t id;        // 与头部id对应
    uint32_t index;     // 块索引 (0-based)
    uint32_t data_size; // 当前块实际数据大小
    uint8_t *data;      // 可变长度数据
  };
  virtual ~FileSyncEngineInterface() = default;
};

#endif