#pragma once

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <memory>
#include <vector>

namespace webserver
{

class EventLoopThreadPool {
public:
  EventLoopThreadPool(EventLoop *base_loop);
  ~EventLoopThreadPool();
  void set_thread_num(int num_threads) { num_threads_ = num_threads; }
  void start();
  EventLoop* get_next_loop();
  bool is_started() { return started_; }

private:
  EventLoop *base_loop_;
  int num_threads_;
  int next_;
  bool started_{false};
  std::vector<EventLoop*> loops_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
};
  
}