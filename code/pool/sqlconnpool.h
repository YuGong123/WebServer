/*
 * @Author       : mark
 * @Date         : 2020-06-16
 * @copyleft Apache 2.0
 */ 
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool *Instance();     // 单例，静态化的实例。

    MYSQL *GetConn();               // 得到连接
    void FreeConn(MYSQL * conn);    // 释放连接，把它重新放回池子里
    int GetFreeConnCount();         // 获取空闲的数量

    void Init(const char* host, int port,           // 主机，端口
              const char* user,const char* pwd,     // 用户，密码
              const char* dbName, int connSize);    // 数据库名，连接数量
    void ClosePool();               // 关闭池子

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;      // 数据库连接池一开始默认创建10个连接，最大连接数就是10个。
    int useCount_;      // 有几个用户连接到mysql
    int freeCount_;     // 空闲的用户数

    std::queue<MYSQL *> connQue_;   // 队列，连接池
    std::mutex mtx_;                // 互斥锁
    sem_t semId_;                   // 信号量
};


#endif // SQLCONNPOOL_H