#pragma once

#include <unordered_map>
#include <poll.h>
#include <signal.h>

namespace webserver
{

class EventLoop;
class Channel;

class Poller {
public:
  // typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();
  
  void poll(std::vector<Channel*> &active_channels, int timeout = -1);
  void fill_active_channels(int num_revents, std::vector<Channel*> &active_channels);
  void update_channel(Channel* new_channel);
  void assert_in_loop_thread() { loop_->assert_in_loop_thread(); }

private:
  EventLoop* loop_;
  std::vector<pollfd> poll_fds_;
  std::unordered_map<int, Channel*> channel_map_;
};
    
}