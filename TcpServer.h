#pragma once

#include "Acceptor.h"
#include "Callbacks.h"
#include "Logging.h"
#include "TcpConnection.h"

#include <unordered_map>

namespace webserver
{

class TcpServer {
public:
  typedef std::unordered_map<std::string, std::shared_ptr<TcpConnection>> ConnectionMap;

  TcpServer(EventLoop* loop, const InetAddress &addr);
  // start() is thread safe, while Acceptor::listen() must be called in loop thread
  void start();
  void new_connection_callback(int connfd, const InetAddress &peer_addr);
  void set_connection_callback(const ConnectionCallback &cb) { connection_cb_ = cb; }
  void set_message_callback(const MessageCallback &cb) { message_cb_ = cb; }
  void set_write_complete_callback(const WriteCompleteCallback &cb) { write_complete_cb_ = cb; }
  /**
   * - 0 means all I/O in loop's thread, no thread will created. This is the default value.
   * - 1 means all I/O in another thread.
   * - N means a thread pool with N threads, new connections are assigned on a round-robin basis.
   */
  void set_thread_num(int num_threads);

private:
  void remove_connection(const std::shared_ptr<TcpConnection> &connection);
  void remove_connection_in_loop(const std::shared_ptr<TcpConnection> &connection);

  bool is_start_{false};
  int next_connection_id_{1};
  EventLoop* loop_;
  InetAddress addr_;
  std::unique_ptr<Acceptor> acceptor_;
  std::unique_ptr<EventLoopThreadPool> thread_pool_;
  ConnectionMap connection_map_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  WriteCompleteCallback write_complete_cb_;
};
  
}