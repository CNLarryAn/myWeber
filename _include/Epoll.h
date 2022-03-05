#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"

//封装了epoll的函数，并且里面的文件描述符用channel类来包装了
class Epoll {
 public:
  Epoll();
  ~Epoll();
  void epoll_add(SP_Channel request, int timeout);
  void epoll_mod(SP_Channel request, int timeout);
  void epoll_del(SP_Channel request);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  void add_timer(std::shared_ptr<Channel> request_data, int timeout);
  int getEpollFd() { return epollFd_; }
  void handleExpired();

 private:
  static const int MAXFDS = 100000;
  int epollFd_;
  //用来记录待处理的事件（在epoll_wait函数里作为参数，函数会将可以处理的事件回传到vector），再转换成SP_channel的vector供其他函数处理
  std::vector<epoll_event> events_;
  //fd到channel的映射
  std::shared_ptr<Channel> fd2chan_[MAXFDS];

  //fd到HttpData的映射
  std::shared_ptr<HttpData> fd2http_[MAXFDS];
  TimerManager timerManager_;
};