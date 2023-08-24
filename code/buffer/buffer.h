/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);    // 初始化buffer容器的大小
    ~Buffer() = default;

    size_t WritableBytes() const;       // 可写的字节数
    size_t ReadableBytes() const ;      // 可读的字节数
    size_t PrependableBytes() const;    // 在前面可追加的字节数

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);        // 写进去

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();                  // buffer_的开始指针
    const char* BeginPtr_() const;      // 重载的BeginPtr_
    void MakeSpace_(size_t len);        // 默认大小是1024个字节，如果不够就创建新的空间，自动增长的缓冲区。

    std::vector<char> buffer_;              // 装具体数据的vector
    std::atomic<std::size_t> readPos_;      // buffer_可读的位置
    std::atomic<std::size_t> writePos_;     // buffer_可写的位置
};

#endif //BUFFER_H