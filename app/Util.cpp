#include "Util.h"
#include <unistd.h>

ssize_t writen(int fd, std::string &sbuff) {
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0) {//ET模式下，只要可写，就一直写，直到数据发送完，或者 errno = EAGAIN
        //调用write之后只是说将用户进程的数据拷贝到了内核里的socket buffer，然后就没write的事了，内核会用自己的进程调用TCP/IP协议栈，把数据发出去
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {//写的过程遇到中断，返回-1并置errno为EINTR
                    nwritten = 0;
                    continue;
                } 
                else if (errno == EAGAIN)//写缓存已满，等会再来写
                    continue;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}