#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "Logging.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      poller_(new Epoll()),
      wakeupFd_(createEventfd()),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      pwakeupChannel_(new Channel(this, wakeupFd_)) {
        if (t_loopInThisThread) {
          // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
        } 
        else {
          t_loopInThisThread = this;
        }
        // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
        pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
        pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
        pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
        poller_->epoll_add(pwakeupChannel_, 0);
      }

void EventLoop::handleConn() {
  // poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET |
  // EPOLLONESHOT), 0);
  updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  // wakeupChannel_->disableAll();
  // wakeupChannel_->remove();
  close(wakeupFd_);
  t_loopInThisThread = NULL;
}
//通过往wakeupfd中写入，来唤醒loop
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb) {
    if (isInLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    if (!isInLoopThread() || callingPendingFunctors_) wakeup(); //doPendingFunctors过程中也要wakeup的原因是，functor可能也会调用queueInLoop，
                                                                //如果不wakeup，在执行完doPendingFunctors之后，循环可能会阻塞在poll上，刚加进去的任务无法及时的被处理
}

//事件循环
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  // LOG_TRACE << "EventLoop " << this << " start looping";
  std::vector<SP_Channel> ret;
  while (!quit_) {
    // cout << "doing" << endl;
    ret.clear();
    ret = poller_->poll();
    eventHandling_ = true;
    for (auto& it : ret) it->handleEvents();
    eventHandling_ = false;
    doPendingFunctors();
    poller_->handleExpired();
  }
  looping_ = false;
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_); //不直接一个个处理，而是先swap到局部变量中，这样可以缩小临界区，不阻塞其他线程调用queueInLoop，也避免了死锁（因为Functor也可能再调用queueInLoop）
  }

  for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  callingPendingFunctors_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}