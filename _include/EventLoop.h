#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "CurrentThread.h"
#include "Logging.h"
#include "Thread.h"


#include <iostream>
using namespace std;

//一个Eventloop变量就代表一个Reactor，是基于Reactor的核心类，主要作用就是维护一个epoll队列，并阻塞监听事件的发生，还封装了几个定时器的函数。
class EventLoop {
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();
  void loop();
  void quit();

  void runInLoop(Functor&& cb);
  void queueInLoop(Functor&& cb);
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  void assertInLoopThread() { assert(isInLoopThread()); }

  void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }

  //每个EventLoop中有一个Epoll，往里面注册、修改、删除channel
  void removeFromPoller(shared_ptr<Channel> channel) {
    // shutDownWR(channel->getFd());
    poller_->epoll_del(channel);
  }
  void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_mod(channel, timeout);
  }
  void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_add(channel, timeout);
  }

 private:
  bool looping_;
  unique_ptr<Epoll> poller_; //是否应该用unique_ptr？
  bool quit_;
  
  bool eventHandling_;
  mutable MutexLock mutex_;
  vector<Functor> pendingFunctors_; //多个线程会往里面放任务让IO线程去完成，需要用锁保护
  bool callingPendingFunctors_;
  const pid_t threadId_;
  // 声明顺序 wakeupFd_ > pwakeupChannel_
  //每个EventLoop维护一个Eventfd，为了唤醒loop
  int wakeupFd_;
  shared_ptr<Channel> pwakeupChannel_;

  void wakeup();
  void handleRead();
  void doPendingFunctors();
  void handleConn();
};
