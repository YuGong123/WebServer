/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;                                 // 文件描述符
    TimeStamp expires;                      // 超时时间
    TimeoutCallBack cb;                     // 回调函数，超时调用CloseConn_
    bool operator<(const TimerNode& t) {    // 重载小于号
        return expires < t.expires;
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);                        // 删除节点
    
    void siftup_(size_t i);                     // 往上调整

    bool siftdown_(size_t index, size_t n);     // 往下调整

    void SwapNode_(size_t i, size_t j);         // 交换两个节点

    std::vector<TimerNode> heap_;               // 用容器实现，不用二叉树实现。

    std::unordered_map<int, size_t> ref_;       // 文件描述符和索引对应的关系。
};

#endif //HEAP_TIMER_H