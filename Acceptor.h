#pragma once

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"
#include "Socket.h"

namespace webserver
{

class Acceptor {
public:
  Acceptor(EventLoop *loop, InetAddress *addr);
  void listen();
  void handle_read();
  void set_connection_callback(const NewConnectionCallback &cb) { connection_callback_ = cb; }
  bool is_listening() { return is_listening_; }
  static void default_connection_callback(int connfd, const InetAddress &peer_addr);

private:
  bool is_listening_{false};
  EventLoop *loop_;
  Socket socket_;
  Channel channel_;
  NewConnectionCallback connection_callback_;
};
  
}