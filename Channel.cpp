#include "Channel.h"

namespace webserver
{

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd) : loop_(loop), fd_(fd) {}

void Channel::handle_event() {
  if (revents_ & none_events_) {
    // LOGWARN << "Channel::handle_event() POLLNVAL";
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
  
}