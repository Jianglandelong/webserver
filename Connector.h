#pragma once

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

namespace webserver
{

class Connector {
public:
  typedef std::function<void(int sockfd)> NewConnectionCallback;
  Connector(EventLoop *loop, const InetAddress &server_addr);
  ~Connector();
  void start();
  void restart();
  void stop();
  void set_new_connection_callback(const NewConnectionCallback &cb) { new_connection_cb_ = cb; }
  const InetAddress& server_addr() const { return server_addr_; }

private:
  enum State {Disconnected, Connecting, Connected};
  static const int max_retry_delay_ms = 30 * 1000;
  static const int init_retry_delay_ms = 500;

  void start_in_loop();
  void connect();
  void init_connection(int sockfd);
  void retry(int sockfd);
  void handle_write();
  void handle_error();
  int remove_and_reset_channel();
  void reset_channel();
  void cancel_timer();

  State state_;
  bool is_start_;
  EventLoop *loop_;
  std::unique_ptr<Channel> channel_;
  InetAddress server_addr_;
  int retry_delay_ms_;
  std::shared_ptr<Timer> retry_timer_;
  NewConnectionCallback new_connection_cb_;
};
  
}