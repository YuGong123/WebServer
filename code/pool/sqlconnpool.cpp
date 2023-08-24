/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 

#include "sqlconnpool.h"
using namespace std;

SqlConnPool::SqlConnPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::Init(const char* host, int port,
            const char* user,const char* pwd, const char* dbName,
            int connSize = 10) {
    assert(connSize > 0);

    // 数据库连接池一开始默认创建10个连接。
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);      // 初始化
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.push(sql);         // 入队列
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);        // 信号量初始值为10，wait, 取一个信号量值减1；post： +1
}

MYSQL* SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    // 数据库空闲连接为0
    if(connQue_.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;             
    }

    // 对信号量加锁，调用一次对信号量的值-1，如果值为0，就阻塞
    sem_wait(&semId_);              
    {
        lock_guard<mutex> locker(mtx_);
        sql = connQue_.front();     // 拿出第一个
        connQue_.pop();             // 移除第一个
    }
    return sql;
}

void SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx_);
    connQue_.push(sql);             // 释放，就是再放回池子里
    sem_post(&semId_);              // 信号量+1
}


// 先清空连接池，再关闭数据库。
void SqlConnPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);
    while(!connQue_.empty()) {
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);          // 关闭数据库的一个连接
    }
    mysql_library_end();        
}

int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQue_.size();
}

SqlConnPool::~SqlConnPool() {
    ClosePool();
}
