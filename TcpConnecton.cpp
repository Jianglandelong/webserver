#include "TcpConnection.h"

namespace webserver
{

TcpConnection::TcpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &server_addr, 
  const InetAddress &peer_addr) :
  loop_(loop), socket_(std::make_unique<Socket>(connfd)), name_(name),
  channel_(std::make_unique<Channel>(loop, connfd)), server_addr_(server_addr), peer_addr_(peer_addr)
{
  channel_->set_read_callback([this]() {this->handle_read(); });
}
  
void TcpConnection::connection_established() {
  loop_->assert_in_loop_thread();
  assert(!is_connected());
  state_ = State::connected;
  channel_->enable_reading();
  connection_cb_(shared_from_this());
}

void TcpConnection::handle_read() {
  char buf[BUFSIZ];
  auto n = ::read(socket_->fd(), buf, sizeof(buf));
  message_cb_(shared_from_this(), buf, n);
}
  
}