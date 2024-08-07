#include <sys/eventfd.h>

#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Poller.h"
#include "TimerQueue.h"

namespace webserver
{

__thread EventLoop* t_loop_in_this_thread = nullptr;
const int timeout = 10000;
IgnoreSigPipe init_obj;

static int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    // LOG_SYSERR << "Failed in eventfd";
    std::cerr << "createEventfd() error: " << std::strerror(errno) << std::endl;
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop() 
  : is_looping_(false),
    is_quit_(false),
    is_calling_pending_function_(false),
    thread_id_(CurrentThread::tid()), 
    wakeup_fd_(createEventfd()),
    poller_(std::make_unique<Poller>(this)), 
    timer_queue_(std::make_unique<TimerQueue>(this))
{
  if (t_loop_in_this_thread != nullptr) {
    LOG << "Another EventLoop " << t_loop_in_this_thread << " exists in this thread " << thread_id_ << "\n";
  } else {
    t_loop_in_this_thread = this;
  }
  wakeup_channel_ = std::make_shared<Channel>(this, wakeup_fd_);
  wakeup_channel_->set_read_callback([this](Timestamp receive_time) { this->handle_read(); });
  wakeup_channel_->enable_reading();
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
    poll_return_time_ = poller_->poll(active_channels_, timeout);
    for (auto &channel : active_channels_) {
      channel->handle_event(poll_return_time_);
    }
    run_pending_functions();
  }
  // LOG_TRACE << "EventLoop " << this << " stop looping";
  LOG << "EventLoop " << this << " stop looping\n";
  is_looping_ = false;
}

void EventLoop::quit() {
  is_quit_ = true;
  if (!is_in_loop_thread()) {
    wakeup();
  }
}

void EventLoop::update_channel(Channel* channel) {
  assert(channel->eventloop() == this);
  assert_in_loop_thread();
  poller_->update_channel(channel);
}

std::shared_ptr<Timer> EventLoop::run_at(const Timestamp &time, const Timer::TimerCallback &cb) {
  return timer_queue_->add_timer(time, cb);
}

std::shared_ptr<Timer> EventLoop::run_after(double delay, const Timer::TimerCallback &cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return run_at(time, cb);
}

std::shared_ptr<Timer> EventLoop::run_every_interval(double interval, const Timer::TimerCallback &cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timer_queue_->add_timer(time, cb, interval);
}

void EventLoop::run_in_loop(const std::function<void()> &cb) {
  if (is_in_loop_thread()) {
    cb();
  }
  else {
    queue_in_loop(cb);
  }
}

void EventLoop::queue_in_loop(const std::function<void()> &cb) {
  mutex_.lock();
  function_list_.push_back(cb);
  mutex_.unlock();
  if (!is_in_loop_thread() || is_calling_pending_function_) {
    wakeup();
  }
}

void EventLoop::remove_channel(Channel *channel) {
  assert(channel->eventloop() == this);
  assert_in_loop_thread();
  poller_->remove_channel(channel);
}

void EventLoop::handle_read() {
  uint64_t one;
  auto num = ::read(wakeup_fd_, &one, sizeof(one));
  if (num != sizeof(one)) {
    LOG_ERROR << "EventLoop::handle_read() read " << num << " bytes instead of 8" << std::endl;
  }
}

void EventLoop::wakeup() {
  uint64_t one;
  auto num = ::write(wakeup_fd_, &one, sizeof(one));
  if (num != sizeof(one)) {
    LOG_ERROR << "EventLoop::wakeup() write " << num << " bytes instead of 8" << std::endl;
  }
}

void EventLoop::run_pending_functions() {
  std::vector<std::function<void()>> tmp_list;
  is_calling_pending_function_ = true;
  mutex_.lock();
  tmp_list.swap(function_list_);
  mutex_.unlock();
  for (auto &functor : tmp_list) {
    functor();
  }
  is_calling_pending_function_ = false;
}

void EventLoop::cancel_timer(std::shared_ptr<Timer> &&timer) {
  timer_queue_->cancel(std::move(timer));
}

}