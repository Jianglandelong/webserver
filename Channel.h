#pragma once

#include "Timer.h"

#include <cassert>
#include <functional>
#include <poll.h>
#include <signal.h>

namespace webserver
{

class EventLoop;

class Channel {
public:
  typedef std::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);

  ~Channel();

  void handle_event(Timestamp receive_time);

  void set_read_callback(const ReadEventCallback &cb) { read_callback_ = cb; }
  void set_write_callback(const std::function<void()> &cb) { write_callback_ = cb; }
  void set_error_callback(const std::function<void()> &cb) { error_callback_ = cb; }
  void set_close_callback(const std::function<void()> &cb) { close_callback_ = cb; }

  int fd() { return fd_; }
  int events() { return events_; }
  int index_in_pollfds() { return index_in_pollfds_; }
  void set_index(int index) { index_in_pollfds_ = index; }
  void set_revents(int revents) { revents_ = revents; }
  bool is_none_event() { return events_ == none_events_; }
  bool is_writing();

  void enable_reading();
  void enable_writing();
  void disable_events();
  void disable_writing();
  void disable_all();

  EventLoop* eventloop() { return loop_; }

private:
  // Call EventLoop::update_channel(), which will call Poller::update_channel()
  void update();

  static const int none_events_{0};
  // static const int read_events_{POLLIN | POLLPRI | POLLRDHUP};
  static const int read_events_{POLLIN | POLLPRI};
  static const int write_events_{POLLOUT};

  int fd_;
  int events_{0};
  int revents_{0};
  int index_in_pollfds_{-1};
  EventLoop* loop_;
  bool handling_event_{false};
  ReadEventCallback read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> error_callback_;
  std::function<void()> close_callback_;
};
  
}