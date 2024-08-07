#pragma once

#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"

#include <map>
#include <set>

namespace webserver
{

class TimerQueue {
public:
  typedef std::function<void()> TimerCallback;

  TimerQueue(EventLoop *loop);
  ~TimerQueue();

  void handle_read();
  void get_expired(Timestamp now);
  void reset_expired(Timestamp now);
  bool insert(std::shared_ptr<Timer> timer);
  std::shared_ptr<Timer> add_timer(Timestamp expiration, TimerCallback cb, double interval = 0.0);
  void update_timer_queue_fd();
  void cancel(std::shared_ptr<Timer> &&timer);

private:
  void add_timer_in_loop(std::shared_ptr<Timer> timer);
  void cancel_in_loop(std::shared_ptr<Timer> &&timer);
  
  int timer_queue_fd_;
  bool is_calling_expired_timers_{false};
  std::set<std::pair<Timestamp, std::shared_ptr<Timer>>> timer_list_;
  Channel timer_queue_channel_;
  EventLoop *loop_;
  std::vector<std::shared_ptr<Timer>> expired_;
  std::set<std::shared_ptr<Timer>> wait_cancel_;
};
  
}