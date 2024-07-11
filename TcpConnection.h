#pragma once

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <memory>

namespace webserver
{

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &server_addr, const InetAddress &peer_addr);
  EventLoop* get_loop() { return loop_; }
  bool is_connected() { return state_ == State::connected; }
  const std::string& name() { return name_; }
  const InetAddress& server_addr() { return server_addr_; }
  const InetAddress& peer_addr() { return peer_addr_; }
  void set_connection_callback(const ConnectionCallback &cb) { connection_cb_ = cb; }
  void set_message_callback(const MessageCallback &cb) { message_cb_ = cb; }
  void connection_established();

private:
  enum State {connecting, connected};
  void handle_read();

  EventLoop *loop_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  State state_;
  InetAddress server_addr_;
  InetAddress peer_addr_;
  std::string name_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
};
  
}