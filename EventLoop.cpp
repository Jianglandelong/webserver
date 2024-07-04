#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Poller.h"
#include "TimerQueue.h"

namespace webserver
{

__thread EventLoop* t_loop_in_this_thread = nullptr;
const int timeout = 10000;

EventLoop::EventLoop() 
  : is_looping_(false),
    is_quit_(false),
    is_calling_pending_function_(false),
    thread_id_(CurrentThread::tid()), 
    poller_(std::make_unique<Poller>(this)), 
    timer_queue_(std::make_unique<TimerQueue>(this))
{
  if (t_loop_in_this_thread != nullptr) {
    LOG << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread " << thread_id_ << "\n";
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
  LOG << "EventLoop " << this << " stop looping\n";
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

void EventLoop::run_at(const Timestamp &time, const Timer::TimerCallback &cb) {
  timer_queue_->add_timer(time, cb);
}

void EventLoop::run_after(double delay, const Timer::TimerCallback &cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  run_at(time, cb);
}

void EventLoop::run_every_interval(double interval, const Timer::TimerCallback &cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  timer_queue_->add_timer(time, cb, interval);
}

}