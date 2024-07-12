#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"

namespace webserver
{

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {}

Channel::~Channel() {
  assert(!handling_event_);
}

void Channel::handle_event() {
  handling_event_ = true;
  if (revents_ & none_events_) {
    LOG_WARN << "Channel::handle_event() POLLNVAL\n";
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) {
      error_callback_();
    }
  }
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG_WARN << "Channel::handle_event() POLLHUP\n";
    if (close_callback_) {
      close_callback_();
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
  handling_event_ = false;
}

void Channel::enable_reading() {
  events_ = read_events_;
  update();
}

void Channel::update() {
  loop_->update_channel(this);
}
  
void Channel::disable_events() {
  events_ = none_events_;
  update();
}
  
}