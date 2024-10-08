#pragma once

#include "CurrentThread.h"
#include "Timer.h"

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace webserver
{
class Channel;
class EPoller;
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
  std::shared_ptr<Timer> run_at(const Timestamp &time, const Timer::TimerCallback &cb);
  std::shared_ptr<Timer> run_after(double delay, const Timer::TimerCallback &cb);
  std::shared_ptr<Timer> run_every_interval(double interval, const Timer::TimerCallback &cb);
  void run_in_loop(const std::function<void()> &cb);
  void queue_in_loop(const std::function<void()> &cb);
  void remove_channel(Channel *channel);
  bool is_calling_pending_functions() { return is_calling_pending_function_; }
  void cancel_timer(std::shared_ptr<Timer> &&timer);

private:
  void handle_read();
  void wakeup();
  void run_pending_functions();

  bool is_looping_;
  bool is_quit_;
  bool is_calling_pending_function_;
  // const pid_t thread_id_;
  pid_t thread_id_;
  int wakeup_fd_;
  std::shared_ptr<Channel> wakeup_channel_;
  std::vector<Channel*> active_channels_;
  std::vector<std::function<void()>> function_list_;
  // std::unique_ptr<Poller> poller_;
  std::unique_ptr<EPoller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  Timestamp poll_return_time_;
  std::mutex mutex_;
};

class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

}