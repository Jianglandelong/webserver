#pragma once

#include "base/CurrentThread.h"

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace webserver
{
class Channel;
class Poller;

class EventLoop {
public:
  EventLoop();

  ~EventLoop();

  void loop();

  void quit();

  void run_in_loop(std::function<void> cb);

  void queue_in_loop(std::function<void> cb);

  bool is_in_loop_thread();

  void assert_in_loop_thread();

  void update_channel();

  void remove_channel();

private:
  void handle_read();

  void run_pending_functions();

  bool is_looping_{false};
  bool is_quit_{false};
  bool is_calling_pending_function_{false};
  const pid_t thread_id_;
  std::vector<std::shared_ptr<Channel>> channel_list_;
  std::vector<std::function<void>> function_list_;
  std::unique_ptr<Poller> poller_;
};
}