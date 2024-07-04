#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"

namespace webserver
{

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {}

Channel::~Channel() {}

void Channel::handle_event() {
  if (revents_ & none_events_) {
    // LOGWARN << "Channel::handle_event() POLLNVAL";
    LOG << "Channel::handle_event() POLLNVAL\n";
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) {
      error_callback_();
    }
  }
  if (revents_ & read_events_) {
    if (read_callback_) {
      read_callback_();
    }
  }
  if (revents_ & write_events_) {
    if (write_callback_) {
      write_callback_();
    }
  }
}

void Channel::enable_reading() {
  events_ = read_events_;
  update();
}

void Channel::update() {
  loop_->update_channel(this);
}
  
}