#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据  Poller工作在LT模式 一直读直到读完
 * Buffer缓冲区是有大小的！ 但是从fd上读数据的时候，却不知道tcp数据最终的大小
 * 如果一开始就把buffer存数据的vector给申请很大的内存,保证可以存下接收的数据,但是就会存在很大的空间浪费
 * 就使用extrabuf栈内存,把buffer存不下的数据暂时放这里,再把vector扩容为extrabuf的数据大小,最后extrabuf离开作用域就会自动释放内存,不浪费一点空间
 */
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536] = {0}; // 栈上的内存空间  64K

    struct iovec vec[2]; // 先读到vec[0],不够会自动读到vec[1]

    const size_t writable = writableBytes(); // 这是Buffer底层缓冲区剩余的可写空间大小
    // 第一块缓冲区
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    // 第二块缓冲区
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1; // 一次最多读64k
    const ssize_t n = ::readv(fd, vec, iovcnt);              // 数据自动读入vec 先填充vec[0] 填满再填vec[1]
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // Buffer的可写缓冲区已经够存储读出来的数据了
    {
        writerIndex_ += n;
    }
    else // extrabuf里面也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); // writerIndex_开始写 n - writable大小的数据 readFd()函数结束后会释放extrabuf,所以要将extrabuf里的数据加入buffer_
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}