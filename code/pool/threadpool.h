/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>        // 容器，工作队列
#include <thread>       // C++11的线程库
#include <functional>   // 回调相关
class ThreadPool {
public:                     
    // explicit：防止构造函数出现隐式转换。比如 A  a = 8,这样是不允许的，只能用下面我们写的构造函数
    // Cat cc = Cat(); —>首先生成了一个匿名对象，然后将此匿名对象变为了cc对象，其生命周期就变成了cc对象的生命周期。
    // Pool *pool_ = new Pool();    shared_ptr<Pool> pool_(new Pool);
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {      
            assert(threadCount > 0);

            // 创建threadCount个子线程
            for(size_t i = 0; i < threadCount; i++) {

                // 一个线程要执行的逻辑
                std::thread([pool = pool_] {                                // 给pool传参。  pool->mtx != pool_->mtx
                    std::unique_lock<std::mutex> locker(pool->mtx);         // 创建一个锁，locker，但是不锁。后面手动调用lock()、unlock函数，加锁、解锁。
                    while(true) {
                        // 如果任务队列不为空，该子线程才能去取第一个任务
                        if(!pool->tasks.empty()) {                          
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();                              // 队列弹出已经被获取的任务
                            locker.unlock();                                // 解锁，pool->mtx 是任务执行时的锁。
                            task();                                         // 执行任务
                            locker.lock();                                  // 上锁
                        } 
                        // 如果工作队列为空,并且池子已经关闭,该子线程就跳出while()循环，该子线程就执行完了，自动销毁。
                        else if(pool->isClosed) break;

                        // 如果线程池为空，也没有关闭，该子线程就阻塞等待任务的添加。
                        else pool->cond.wait(locker);                       // 解锁， 阻塞等待条件变量满足  --》重新加锁
                    }
                }).detach();        // std::thread().detach();      设置线程分离
            }
    }

    ThreadPool() = default;                 // 采用默认的构造函数

    ThreadPool(ThreadPool&&) = default;     // 采用默认的拷贝构造函数
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);     // std::lock_guard在构造函数里调用互斥体的lock函数进行加锁,
                pool_->isClosed = true;                             // std::lock_guard在析构函数里调用互斥体的unlock函数进行解锁。
            }
            pool_->cond.notify_all();            // 唤醒线程池中所有的线程,线程最终就会进入  else if(pool->isClosed) break; 逻辑,最后退出
        }
    }


    // F 是task任务的类型
    template<class F> 
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);     // 加pool_->mtx锁, pool_->mtx 是添加任务时的锁。 
            pool_->tasks.emplace(std::forward<F>(task));        // 向工作队列中插入一个任务
            // emplace() 是 C++ 11 标准新增加的成员函数，用于在 vector 容器指定位置之前插入一个新的元素。 再次强调，emplace() 每次只能插入一个元素，而不是多个
        }
        pool_->cond.notify_one();                               // 唤醒一个线程  条件变量+1
    }

private:
    // 定义了一个池子
    struct Pool {
        std::mutex mtx;                             // 互斥锁
        std::condition_variable cond;               // 条件变量
        bool isClosed;                              // 是否关闭
        std::queue<std::function<void()>> tasks;    // 工作任务队列
    };
    std::shared_ptr<Pool> pool_;        // 定义了一个线程池指针变量，shared_ptr是智能指针，代码可改变为： Pool *pool;
};


#endif //THREADPOOL_H