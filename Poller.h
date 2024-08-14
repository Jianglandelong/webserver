#pragma once

#include "Timer.h"

#include <unordered_map>
#include <poll.h>
#include <signal.h>
#include <sys/epoll.h>

namespace webserver
{

class EventLoop;
class Channel;

class Poller {
public:
  // typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();
  
  Timestamp poll(std::vector<Channel*> &active_channels, int timeout = -1);
  void fill_active_channels(int num_revents, std::vector<Channel*> &active_channels);
  void update_channel(Channel *new_channel);
  void remove_channel(Channel *channel);
  void assert_in_loop_thread() { loop_->assert_in_loop_thread(); }

private:
  EventLoop* loop_;
  std::vector<pollfd> poll_fds_;
  std::unordered_map<int, Channel*> channel_map_;
};

class EPoller {
public:
  EPoller(EventLoop *loop);
  ~EPoller();

  Timestamp poll(std::vector<Channel*> &active_channels, int timeout = -1);
  void update_channel(Channel *new_channel);
  void remove_channel(Channel *channel);

private:
  void fill_active_channels(int num_events, std::vector<Channel*> &active_channels);

  EventLoop *loop_;
  int epfd_;
  std::vector<epoll_event> epoll_events_;
  std::unordered_map<int, Channel*> channel_map_;
  static const int init_epoll_events_size_ = 64;
};
    
}