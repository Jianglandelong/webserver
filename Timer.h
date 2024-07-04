#pragma once

#include "base/Timestamp.h"

#include <functional>

namespace webserver
{

class Timer {
public:
  typedef std::function<void()> TimerCallback;

  Timer() : interval_(0.0) {}
  Timer(Timestamp expiration, TimerCallback cb, double interval = 0.0)
    : expiration_(expiration), cb_(cb), interval_(interval), is_repeat_(interval > 0)
  {}

  void callback() { cb_(); }

  Timestamp expiration() const  { return expiration_; }
  bool is_repeat() const { return is_repeat_; }

  void restart(Timestamp now) {
    if (is_repeat_) {
      expiration_ = addTime(now, interval_);
    } else {
      expiration_ = Timestamp::invalid();
    }
  }

private:
  Timestamp expiration_;
  bool is_repeat_;
  const double interval_;
  const TimerCallback cb_;
};
  
}