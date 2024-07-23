#pragma once

#include "Buffer.h"
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
  void set_close_callback(const CloseCallback &cb) { close_cb_ = cb; }
  void set_write_complete_callback(const WriteCompleteCallback &cb) { write_complete_cb_ = cb; }
  void initialize_connection();
  void destroy_connection();
  void send(const std::string &message);
  void shutdown();
  void set_tcp_no_delay(bool on);

private:
  enum State {connecting, connected, disconnecting, disconnected};
  void handle_read(Timestamp receive_time);
  void handle_close();
  void handle_error();
  void handle_write();
  void send_in_loop(const std::string &message);
  void shutdown_in_loop();

  EventLoop *loop_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  State state_;
  InetAddress server_addr_;
  InetAddress peer_addr_;
  std::string name_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  CloseCallback close_cb_;
  WriteCompleteCallback write_complete_cb_;
};
  
}