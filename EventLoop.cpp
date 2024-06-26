#include "EventLoop.h"
#include "Poller.h"

namespace webserver
{

__thread EventLoop* t_loop_in_this_thread = nullptr;

EventLoop::EventLoop() 
  : thread_id_(std::this_thread::get_id()), 
    poller_(new Poller(this)),
    thread_id_(CurrentThread::tid())
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

  ::poll(nullptr, 0, 3*1000);
  is_looping_ = false;
}

}