#pragma once

#include "base/CurrentThread.h"
#include "Timer.h"

#include <cassert>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace webserver
{
class Channel;
class Poller;
class TimerQueue;

class EventLoop {
public:
  // typedef std::function<void()> std::function<void()>;

  EventLoop();

  ~EventLoop();

  void loop();
  void quit();
  bool is_in_loop_thread();
  void assert_in_loop_thread();
  void update_channel(Channel* channel);
  void run_at(const Timestamp &time, const Timer::TimerCallback &cb);
  void run_after(double delay, const Timer::TimerCallback &cb);
  void run_every_interval(double interval, const Timer::TimerCallback &cb);
  void run_in_loop(const std::function<void()> &cb);
  void queue_in_loop(const std::function<void()> &cb);
  void remove_channel();

private:
  void handle_read();
  void run_pending_functions();

  bool is_looping_{false};
  bool is_quit_{false};
  bool is_calling_pending_function_{false};
  const pid_t thread_id_;
  std::vector<Channel*> active_channels_;
  std::vector<std::function<void()>> function_list_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
};
}