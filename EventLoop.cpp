#include "Channel.h"
#include "EventLoop.h"
#include "Poller.h"

namespace webserver
{

__thread EventLoop* t_loop_in_this_thread = nullptr;
const int timeout = 1000;

EventLoop::EventLoop() 
  : thread_id_(CurrentThread::tid()), 
    poller_(std::make_unique<Poller>(this))
{
  if (t_loop_in_this_thread) {
    // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
    // thread " << threadId_;
  } else {
    t_loop_in_this_thread = this;
  }
}

EventLoop::~EventLoop() {
  assert(!is_looping_);
  t_loop_in_this_thread = nullptr;
}

bool EventLoop::is_in_loop_thread() {
  return thread_id_ == CurrentThread::tid();
}

void EventLoop::assert_in_loop_thread() {
  assert(is_in_loop_thread());
}

void EventLoop::loop() {
  assert(!is_looping_);
  assert_in_loop_thread();
  is_looping_ = true;
  while (!is_quit_) {
    active_channels_.clear();
    poller_->poll(active_channels_, timeout);
    for (auto &channel : active_channels_) {
      channel->handle_event();
    }
  }
  // LOG_TRACE << "EventLoop " << this << " stop looping";
  is_looping_ = false;
}

void EventLoop::quit() {
  is_quit_ = true;
  // wakeup();
}

void EventLoop::update_channel(Channel* channel) {
  assert(channel->eventloop() == this);
  assert_in_loop_thread();
  poller_->update_channel(channel);
}

}