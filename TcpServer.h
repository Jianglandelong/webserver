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

private:
  void remove_connection(const std::shared_ptr<TcpConnection> &connection);

  bool is_start_{false};
  int next_connection_id_{1};
  EventLoop* loop_;
  InetAddress addr_;
  std::unique_ptr<Acceptor> acceptor_;
  ConnectionMap connection_map_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
};
  
}