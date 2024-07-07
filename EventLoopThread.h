#include "EventLoop.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace webserver
{

class EventLoopThread {
public:
  EventLoopThread();
  ~EventLoopThread();
  void thread_function();
  EventLoop* start_loop();

private:
  EventLoop *loop_{nullptr};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};
  
}