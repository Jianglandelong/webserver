#pragma once

#include "Connector.h"
#include "EventLoop.h"
#include "TcpConnection.h"

namespace webserver
{

class TcpClient {
public:
  TcpClient(EventLoop *loop, const InetAddress &server_addr);
  ~TcpClient();
  void connect();
  void disconnect();  // connection half shutdown
  void stop();  // connector stop retrying connecting
  auto get_connection() -> std::shared_ptr<TcpConnection> {
    return tcp_connection_;
  }
  void enable_retry() { retry_ = true; }
  void set_connection_callback(const ConnectionCallback &cb_) { connection_cb_ = cb_; }
  void set_message_callback(const MessageCallback &cb_) { message_cb_ = cb_; }
  void set_write_complete_callback(const WriteCompleteCallback &cb_) { write_complete_cb_ = cb_; }

private:
  void new_connection_callback(int sockfd);
  void remove_connection(const std::shared_ptr<TcpConnection> &connection);

  EventLoop *loop_;
  bool retry_;
  bool connect_;
  int next_conn_id_;
  std::shared_ptr<Connector> connector_;
  std::shared_ptr<TcpConnection> tcp_connection_;
  std::mutex mutex_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  WriteCompleteCallback write_complete_cb_;
};
  
}