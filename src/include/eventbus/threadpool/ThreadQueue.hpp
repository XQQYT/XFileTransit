/*
 * ThreadQueue - Fixed Version
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#ifndef THREADQUEUE_H
#define THREADQUEUE_H

#include <iostream>
#include <atomic>
#include <memory>
#include <functional>
#include <tuple>
#include <optional>
#include <vector>
#include <thread>
#include <stdexcept>

static const unsigned int default_capacity = 1024;

// 前向声明
template <class... Args>
class ThreadQueue;

// 危险指针管理器（简化版）
class HazardPointerManager {
private:
    static const int HP_COUNT = 4; // 每个线程最多4个危险指针
    static const int MAX_THREADS = 128; // 最大线程数
    
    struct HazardPointer {
        std::atomic<void*> ptr;
        std::atomic<bool> active;
    };
    
    struct ThreadData {
        HazardPointer hps[HP_COUNT];
        std::atomic<ThreadData*> next;
        
        ThreadData() {
            for (auto& hp : hps) {
                hp.ptr.store(nullptr, std::memory_order_relaxed);
                hp.active.store(false, std::memory_order_relaxed);
            }
            next.store(nullptr, std::memory_order_relaxed);
        }
    };
    
    std::atomic<ThreadData*> head;
    std::vector<void*> retired_list; // 本线程的待回收列表
    
    // 线程局部存储
    static thread_local ThreadData* local_data;
    static thread_local HazardPointerManager* local_manager;
    
public:
    HazardPointerManager() : head(nullptr) {}
    
    ~HazardPointerManager() {
        // 清理所有待回收内存
        for (void* ptr : retired_list) {
            ::operator delete(ptr);
        }
        retired_list.clear();
    }
    
    // 获取当前线程的ThreadData
    ThreadData* get_local_data() {
        if (!local_data) {
            local_data = new ThreadData();
            ThreadData* expected = head.load(std::memory_order_acquire);
            do {
                local_data->next.store(expected, std::memory_order_relaxed);
            } while (!head.compare_exchange_weak(
                expected, local_data, 
                std::memory_order_acq_rel, 
                std::memory_order_acquire));
        }
        return local_data;
    }
    
    // 获取危险指针
    std::atomic<void*>* acquire_hazard_pointer() {
        ThreadData* data = get_local_data();
        for (int i = 0; i < HP_COUNT; ++i) {
            bool expected = false;
            if (data->hps[i].active.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel)) {
                return &data->hps[i].ptr;
            }
        }
        throw std::runtime_error("No available hazard pointers");
    }
    
    // 释放危险指针
    void release_hazard_pointer(std::atomic<void*>* hp) {
        hp->store(nullptr, std::memory_order_release);
        // 不需要重置active，因为acquire时会设置
    }
    
    // 回收内存
    void retire(void* ptr) {
        retired_list.push_back(ptr);
        if (retired_list.size() >= 2 * HP_COUNT * MAX_THREADS) {
            scan();
        }
    }
    
private:
    // 扫描并回收安全的内存
    void scan() {
        // 收集所有活跃的危险指针
        std::vector<void*> hazards;
        ThreadData* current = head.load(std::memory_order_acquire);
        while (current) {
            for (int i = 0; i < HP_COUNT; ++i) {
                if (current->hps[i].active.load(std::memory_order_acquire)) {
                    void* ptr = current->hps[i].ptr.load(std::memory_order_acquire);
                    if (ptr) {
                        hazards.push_back(ptr);
                    }
                }
            }
            current = current->next.load(std::memory_order_acquire);
        }
        
        // 排序以便二分查找
        std::sort(hazards.begin(), hazards.end());
        
        // 分离可以安全回收的指针
        std::vector<void*> safe_to_delete;
        for (void* ptr : retired_list) {
            if (!std::binary_search(hazards.begin(), hazards.end(), ptr)) {
                safe_to_delete.push_back(ptr);
            }
        }
        
        // 从retired_list中移除安全指针
        auto it = std::remove_if(retired_list.begin(), retired_list.end(),
            [&](void* ptr) {
                return std::find(safe_to_delete.begin(), safe_to_delete.end(), ptr) != safe_to_delete.end();
            });
        retired_list.erase(it, retired_list.end());
        
        // 释放内存
        for (void* ptr : safe_to_delete) {
            ::operator delete(ptr);
        }
    }
};

// 初始化线程局部存储
thread_local HazardPointerManager::ThreadData* HazardPointerManager::local_data = nullptr;
thread_local HazardPointerManager* HazardPointerManager::local_manager = nullptr;

// 全局危险指针管理器
static HazardPointerManager g_hp_manager;

template <class... Args>
class ThreadQueue {
private:
    struct Node {
        std::pair<std::function<void(Args...)>, std::tuple<Args...>> data;
        std::atomic<Node*> next;
        
        Node(std::function<void(Args...)>&& func, std::tuple<Args...>&& args)
            : data(std::move(func), std::move(args)), next(nullptr) {}
            
        Node() : next(nullptr) {} // 默认构造函数用于哨兵节点
    };

    // 带标签的指针解决ABA问题
    struct TaggedPtr {
        Node* ptr;
        uintptr_t tag;
        
        TaggedPtr(Node* p = nullptr, uintptr_t t = 0) : ptr(p), tag(t) {}
        
        bool operator==(const TaggedPtr& other) const {
            return ptr == other.ptr && tag == other.tag;
        }
    };

public:
    explicit ThreadQueue(int max) noexcept
        : capacity(max > 0 ? max : default_capacity) 
    {
        // 使用new创建哨兵节点（危险指针会管理其生命周期）
        Node* dummy = new Node();
        head.store(TaggedPtr(dummy, 0), std::memory_order_relaxed);
        tail.store(TaggedPtr(dummy, 0), std::memory_order_relaxed);
        size.store(0, std::memory_order_relaxed);
    }

    ThreadQueue() 
        : ThreadQueue(default_capacity)
    {}

    // 禁止拷贝和移动
    ThreadQueue(const ThreadQueue&) = delete;
    ThreadQueue& operator=(const ThreadQueue&) = delete;
    
    ~ThreadQueue() {
        // 安全的清空队列
        safeClear();
        
        // 删除哨兵节点
        TaggedPtr h = head.load(std::memory_order_relaxed);
        if (h.ptr) {
            // 使用危险指针保护
            auto hp = g_hp_manager.acquire_hazard_pointer();
            hp->store(h.ptr, std::memory_order_release);
            if (head.load(std::memory_order_acquire).ptr == h.ptr) {
                delete h.ptr;
            }
            g_hp_manager.release_hazard_pointer(hp);
        }
    }

    void addTask(std::function<void(Args...)>&& func, Args&&... args) 
    {
        if (size.load(std::memory_order_acquire) >= capacity) {
            throw std::runtime_error("queue is full");
        }
        
        // 使用RAII确保异常安全
        std::unique_ptr<Node> new_node_ptr = 
            std::make_unique<Node>(std::move(func), std::make_tuple(std::forward<Args>(args)...));
        Node* new_node = new_node_ptr.get();
        
        while (true) {
            TaggedPtr tail_ptr = tail.load(std::memory_order_acquire);
            
            // 使用危险指针保护tail节点
            auto hp = g_hp_manager.acquire_hazard_pointer();
            hp->store(tail_ptr.ptr, std::memory_order_release);
            if (tail.load(std::memory_order_acquire).ptr != tail_ptr.ptr) {
                g_hp_manager.release_hazard_pointer(hp);
                continue;
            }
            
            Node* next_ptr = tail_ptr.ptr->next.load(std::memory_order_acquire);
            
            // 验证一致性
            if (tail_ptr.ptr == tail.load(std::memory_order_acquire).ptr) {
                if (next_ptr == nullptr) {
                    // 尝试链接新节点
                    if (tail_ptr.ptr->next.compare_exchange_weak(
                        next_ptr, new_node,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire)) {
                        
                        // 成功添加，尝试移动tail
                        TaggedPtr new_tail(new_node, tail_ptr.tag + 1);
                        tail.compare_exchange_strong(
                            tail_ptr, new_tail,
                            std::memory_order_acq_rel,
                            std::memory_order_acquire);
                        
                        // 释放所有权，节点已加入队列
                        new_node_ptr.release();
                        size.fetch_add(1, std::memory_order_release);
                        g_hp_manager.release_hazard_pointer(hp);
                        return;
                    }
                } else {
                    // 帮助其他线程完成tail更新
                    TaggedPtr new_tail(next_ptr, tail_ptr.tag + 1);
                    tail.compare_exchange_strong(
                        tail_ptr, new_tail,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire);
                }
            }
            g_hp_manager.release_hazard_pointer(hp);
        }
    }

    std::pair<std::function<void(Args...)>, std::tuple<Args...>> getTask() 
    {
        while (true) {
            TaggedPtr head_ptr = head.load(std::memory_order_acquire);
            
            // 使用危险指针保护head节点
            auto hp_head = g_hp_manager.acquire_hazard_pointer();
            hp_head->store(head_ptr.ptr, std::memory_order_release);
            if (head.load(std::memory_order_acquire).ptr != head_ptr.ptr) {
                g_hp_manager.release_hazard_pointer(hp_head);
                continue;
            }
            
            TaggedPtr tail_ptr = tail.load(std::memory_order_acquire);
            Node* next_ptr = head_ptr.ptr->next.load(std::memory_order_acquire);
            
            // 验证一致性
            if (head_ptr.ptr == head.load(std::memory_order_acquire).ptr) {
                if (head_ptr.ptr == tail_ptr.ptr) {
                    if (next_ptr == nullptr) {
                        g_hp_manager.release_hazard_pointer(hp_head);
                        throw std::runtime_error("task queue is empty");
                    }
                    // 帮助推进tail
                    TaggedPtr new_tail(next_ptr, tail_ptr.tag + 1);
                    tail.compare_exchange_strong(
                        tail_ptr, new_tail,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire);
                } else {
                    if (next_ptr == nullptr) {
                        g_hp_manager.release_hazard_pointer(hp_head);
                        continue; // 其他线程正在修改，重试
                    }
                    
                    // 使用危险指针保护next节点
                    auto hp_next = g_hp_manager.acquire_hazard_pointer();
                    hp_next->store(next_ptr, std::memory_order_release);
                    if (head_ptr.ptr->next.load(std::memory_order_acquire) != next_ptr) {
                        g_hp_manager.release_hazard_pointer(hp_next);
                        g_hp_manager.release_hazard_pointer(hp_head);
                        continue;
                    }
                    
                    // 预先复制数据，避免数据竞争
                    auto task_func = next_ptr->data.first;
                    auto task_args = next_ptr->data.second;
                    
                    TaggedPtr new_head(next_ptr, head_ptr.tag + 1);
                    if (head.compare_exchange_weak(
                        head_ptr, new_head,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire)) {
                        
                        // 安全回收旧头节点
                        g_hp_manager.retire(head_ptr.ptr);
                        size.fetch_sub(1, std::memory_order_release);
                        
                        g_hp_manager.release_hazard_pointer(hp_next);
                        g_hp_manager.release_hazard_pointer(hp_head);
                        return std::make_pair(std::move(task_func), std::move(task_args));
                    }
                    
                    g_hp_manager.release_hazard_pointer(hp_next);
                }
            }
            g_hp_manager.release_hazard_pointer(hp_head);
        }
    }

    std::optional<std::pair<std::function<void(Args...)>, std::tuple<Args...>>> tryGetTask() 
    {
        while (true) {
            TaggedPtr head_ptr = head.load(std::memory_order_acquire);
            
            // 使用危险指针保护head节点
            auto hp_head = g_hp_manager.acquire_hazard_pointer();
            hp_head->store(head_ptr.ptr, std::memory_order_release);
            if (head.load(std::memory_order_acquire).ptr != head_ptr.ptr) {
                g_hp_manager.release_hazard_pointer(hp_head);
                continue;
            }
            
            Node* next_ptr = head_ptr.ptr->next.load(std::memory_order_acquire);
            
            if (next_ptr == nullptr) {
                g_hp_manager.release_hazard_pointer(hp_head);
                return std::nullopt;
            }
            
            // 使用危险指针保护next节点
            auto hp_next = g_hp_manager.acquire_hazard_pointer();
            hp_next->store(next_ptr, std::memory_order_release);
            if (head_ptr.ptr->next.load(std::memory_order_acquire) != next_ptr) {
                g_hp_manager.release_hazard_pointer(hp_next);
                g_hp_manager.release_hazard_pointer(hp_head);
                continue;
            }
            
            // 预先读取必要数据
            auto task_func = next_ptr->data.first;
            auto task_args = next_ptr->data.second;
            
            TaggedPtr new_head(next_ptr, head_ptr.tag + 1);
            if (head.compare_exchange_strong(
                head_ptr, new_head,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
                
                // 安全回收旧头节点
                g_hp_manager.retire(head_ptr.ptr);
                size.fetch_sub(1, std::memory_order_release);
                
                g_hp_manager.release_hazard_pointer(hp_next);
                g_hp_manager.release_hazard_pointer(hp_head);
                return std::make_pair(std::move(task_func), std::move(task_args));
            }
            
            g_hp_manager.release_hazard_pointer(hp_next);
            g_hp_manager.release_hazard_pointer(hp_head);
        }
        
        return std::nullopt;
    }

    inline unsigned int getCapacity() noexcept {
        return capacity;
    }

    inline unsigned int getSize() noexcept {
        return size.load(std::memory_order_acquire);
    }

    bool isEmpty() noexcept {
        TaggedPtr head_ptr = head.load(std::memory_order_acquire);
        Node* next_ptr = head_ptr.ptr->next.load(std::memory_order_acquire);
        return next_ptr == nullptr;
    }

    // 安全清空队列
    void clear() {
        safeClear();
    }

private:
    // 安全的清空实现
    void safeClear() {
        while (true) {
            auto task = tryGetTask();
            if (!task.has_value()) {
                break;
            }
        }
    }

private:
    std::atomic<TaggedPtr> head;
    std::atomic<TaggedPtr> tail;
    unsigned int capacity;
    std::atomic<unsigned int> size;
};

#endif // THREADQUEUE_H