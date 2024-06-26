#pragma once

#include "EventLoop.h"

#include <poll.h>
#include <signal.h>

namespace webserver
{

class Channel {
public:
  typedef std::function<void()> EventCallback;

  Channel(std::shared_ptr<EventLoop> loop, int fd);

  ~Channel();

  void handle_event();

  void set_read_callback(const EventCallback &cb) { read_callback_ = cb; }
  void set_write_callback(const EventCallback &cb) { write_callback_ = cb; }
  void set_error_callback(const EventCallback &cb) { error_callback_ = cb; }

  int fd() { return fd_; }
  int events() { return events_; }
  int revents() {return revents_; }

  void enable_reading();
  void enable_writing();

  std::shared_ptr<EventLoop> eventloop() { return loop_; }

private:
  // Call EventLoop::update_channel(), which will call Poller::update_channel()
  void update();

  static const int none_events_{0};
  static const int read_events_{POLLIN | POLLPRI | POLLRDHUP};
  static const int write_events_{POLLOUT};

  int fd_;
  int events_;
  int revents_;
  std::shared_ptr<EventLoop> loop_;
  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback error_callback_;
};
  
}