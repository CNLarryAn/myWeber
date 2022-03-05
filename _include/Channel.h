#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"

class EventLoop;
class HttpData;

//Channel类是最小的事件处理单元，每个Channel会绑定一个fd和一个Eventloop
//如果这个fd发生的事件被监听到，就会调用这个Channel中的回调函数去处理事件
//另外，监听这个fd以及处理这个fd的线程就是绑定的Eventloop的IO线程。
class Channel {
 private:
  typedef std::function<void()> CallBack;
  EventLoop *loop_;
  int fd_;
  __uint32_t events_;//它关心的IO事件，由用户设置
  __uint32_t revents_;//目前活动的事件，由epoll设置
  __uint32_t lastEvents_;

  // 方便找到上层持有该Channel的对象
  std::weak_ptr<HttpData> holder_;

 private:

  //会在handleEvent这个成员函数中根据不同的事件被调用
  CallBack readHandler_;
  CallBack writeHandler_;
  CallBack errorHandler_;
  CallBack connHandler_;

 public:
    Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();
    int getFd();
    void setFd(int fd);

    //设置一个指向上层httpdata的weak_ptr，方便寻找其上层并且防止shared_ptr死锁
    void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }

    std::shared_ptr<HttpData> getHolder() {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

//设置各种回调事件
  void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
  void setWriteHandler(CallBack &&writeHandler) { writeHandler_ = writeHandler; }
  void setErrorHandler(CallBack &&errorHandler) { errorHandler_ = errorHandler; }
  void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }

//根据channel中对应的fd的事件类型，来回调相应的事件
  void handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      events_ = 0;
      return;
    }
    if (revents_ & EPOLLERR) {
      if (errorHandler_) errorHandler_();
      events_ = 0;
      return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      handleRead();
    }
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    handleConn();
  }
  
  //不同事件类型对应的回调函数
  void handleRead();
  void handleWrite();
  void handleError(int fd, int err_num, std::string short_msg);
  void handleConn();


  void setRevents(__uint32_t ev) { revents_ = ev; }

  void setEvents(__uint32_t ev) { events_ = ev; }
  __uint32_t &getEvents() { return events_; }

  bool EqualAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }

  __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;