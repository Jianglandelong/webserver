#include "EventLoopThreadPool.h"

namespace webserver
{

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop) : base_loop_(base_loop), num_threads_(0), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start() {
  assert(!started_);
  base_loop_->assert_in_loop_thread();
  started_ = true;
  for (int i = 0; i < num_threads_; i++) {
    threads_.emplace_back(std::make_unique<EventLoopThread>());
    loops_.emplace_back(threads_[i]->start_loop());
  }
}

EventLoop* EventLoopThreadPool::get_next_loop() {
  base_loop_->assert_in_loop_thread();
  EventLoop* loop = base_loop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    next_++;
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}
  
}