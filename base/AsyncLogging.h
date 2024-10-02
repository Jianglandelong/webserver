#pragma once
#include <functional>
#include <string>
#include <vector>
#include <condition_variable>
#include <optional>
#include <thread>
#include "LogStream.h"

class AsyncLogging {
 public:
  AsyncLogging(const std::string basename, int flushInterval = 2);
  ~AsyncLogging() {
    if (running_) stop();
  }
  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.emplace(&AsyncLogging::threadFunc, this);
    std::unique_lock<std::mutex> lock(mutex_);
    thread_cond_.wait(lock);  // 等待线程启动后再返回 start
    // latch_.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify_all();
    if (thread_.has_value()) {
      thread_.value().join();
    }
  }

 private:
  void threadFunc();
  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  typedef std::shared_ptr<Buffer> BufferPtr;
  const int flushInterval_;
  bool running_;
  std::string basename_;
  // Thread thread_;
  std::optional<std::thread> thread_;
  // MutexLock mutex_;
  std::mutex mutex_;
  // Condition cond_;
  std::condition_variable cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
  // CountDownLatch latch_;
  std::condition_variable thread_cond_;
};