#include "EventLoopThread.h"

namespace webserver
{

EventLoopThread::EventLoopThread() : thread_([this]() { this->thread_function(); }) {}

EventLoopThread::~EventLoopThread() {
  thread_.join();
}

void EventLoopThread::thread_function() {
  EventLoop loop;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cv_.notify_all();
  }
  loop.loop();
}

EventLoop* EventLoopThread::start_loop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (loop_ == nullptr) {
    cv_.wait(lock);
  }
  return loop_;
}
  
}