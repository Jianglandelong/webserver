#include <sys/timerfd.h>

#include "Logging.h"
#include "TimerQueue.h"

namespace webserver
{

int create_timerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    // LOG_SYSFATAL << "Failed in timerfd_create";
    LOG << "Failed in timerfd_create\n";
  }
  return timerfd;
}

void read_timerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
  // LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  LOG << "TimerQueue::handleRead() " << howmany << " at " << now.toString() << "\n";
  if (n != sizeof(howmany))
  {
    // LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    LOG << "TimerQueue::handleRead() reads " << n << " bytes instead of 8\n";
  }
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

TimerQueue::TimerQueue(EventLoop *loop)
  : loop_(loop), timer_queue_fd_(create_timerfd()), timer_queue_channel_(loop, timer_queue_fd_)
{
  timer_queue_channel_.set_read_callback([this](Timestamp receive_time) { this->handle_read(); } );
  timer_queue_channel_.enable_reading();
}

TimerQueue::~TimerQueue() {
  close(timer_queue_fd_);
}

void TimerQueue::handle_read() {
  loop_->assert_in_loop_thread();
  auto now = Timestamp::now();
  read_timerfd(timer_queue_fd_, now);
  get_expired(now);
  for (const auto &timer: expired_) {
    timer->callback();
  }
  reset_expired(now);
}

// Expired timers will be removed from timer_list_
void TimerQueue::get_expired(Timestamp now) {
  auto entry = std::make_pair(now, std::make_shared<Timer>());
  auto iter = timer_list_.upper_bound(entry);
  for (auto it = timer_list_.begin(); it != iter; it++) {
    expired_.push_back(it->second);
  }
  timer_list_.erase(timer_list_.begin(), iter);
}

void TimerQueue::reset_expired(Timestamp now) {
  std::vector<std::shared_ptr<Timer>> tmp_timer_list;
  for (auto &timer : expired_) {
    if (timer->is_repeat()) {
      timer->restart(now);
      tmp_timer_list.push_back(timer);
    }
  }
  for (auto &timer: tmp_timer_list) {
    insert(timer);
  }
  expired_.clear();
  if (!timer_list_.empty()) {
    auto next_expire = timer_list_.begin()->second->expiration();
    if (next_expire.valid()) {
      update_timer_queue_fd();
    }
  } else {
    LOG << "no timer left\n";
  }
}

bool TimerQueue::insert(std::shared_ptr<Timer> timer) {
  bool earliest_changed = false;
  if (timer_list_.empty() || timer->expiration() < timer_list_.begin()->first) {
    earliest_changed = true;
  }
  timer_list_.emplace(timer->expiration(), timer);
  return earliest_changed;
}

void TimerQueue::add_timer(Timestamp expiration, TimerCallback cb, double interval) {
  auto timer = std::make_shared<Timer>(expiration, cb, interval);
  loop_->run_in_loop([this, timer]() { this->add_timer_in_loop(timer); });
}

void TimerQueue::add_timer_in_loop(std::shared_ptr<Timer> timer) {
  loop_->assert_in_loop_thread();
  auto earliest_changed = insert(timer);
  if (earliest_changed) {
    update_timer_queue_fd();
  }
}

void TimerQueue::update_timer_queue_fd()
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof(newValue));
  memset(&oldValue, 0, sizeof(oldValue));
  auto expiration = timer_list_.begin()->first;
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timer_queue_fd_, 0, &newValue, &oldValue);
  if (ret)
  {
    // LOG_SYSERR << "timerfd_settime()";
    std::cerr << "timerfd_settime() error: " << std::strerror(errno) << std::endl;
  }
}

}