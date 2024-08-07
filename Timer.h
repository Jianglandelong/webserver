#pragma once

#include "base/Timestamp.h"

#include <atomic>
#include <functional>

namespace webserver
{

class Timer {
public:
  typedef std::function<void()> TimerCallback;

  Timer() : interval_(0.0), sequence_num_(num_timers_++) {}
  Timer(Timestamp expiration, TimerCallback cb, double interval = 0.0)
    : expiration_(expiration), cb_(cb), interval_(interval), is_repeat_(interval > 0), sequence_num_(num_timers_++)
  {}

  void callback() { cb_(); }

  Timestamp expiration() const  { return expiration_; }
  bool is_repeat() const { return is_repeat_; }
  int64_t sequence() { return sequence_num_; }

  void restart(Timestamp now) {
    if (is_repeat_) {
      expiration_ = addTime(now, interval_);
    } else {
      expiration_ = Timestamp::invalid();
    }
  }

private:
  static std::atomic<int64_t> num_timers_;
  Timestamp expiration_;
  bool is_repeat_;
  const double interval_;
  const TimerCallback cb_;
  int64_t sequence_num_;
};
  
}