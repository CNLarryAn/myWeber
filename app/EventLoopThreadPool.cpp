#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), started_(false), numThreads_(numThreads), next_(0) {
  if (numThreads_ <= 0) {
    LOG << "numThreads_ <= 0";
    abort();
  }
}

/*先判断一下运行该函数的线程是否是主loop(也就是mainreactor)所在的线程
    然后创建所需数目的用智能指针管理的 线程(subreactor)，放入线程池中*/
void EventLoopThreadPool::start() {
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    std::shared_ptr<EventLoopThread> t(new EventLoopThread());
    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop *loop = baseLoop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    next_ = (next_ + 1) % numThreads_;
  }
  return loop;
}