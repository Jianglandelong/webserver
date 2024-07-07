#include "Acceptor.h"

namespace webserver
{

Acceptor::Acceptor(EventLoop *loop, InetAddress *addr) : 
  loop_(loop), socket_(Socket::create_non_blocking_socket()), channel_(loop, socket_.fd()), 
  connection_callback_(default_connection_callback)
{
  socket_.set_reuse_addr(true);
  socket_.bind(addr);
  channel_.set_read_callback([this]() { this->handle_read(); });
}

void Acceptor::listen() {
  loop_->assert_in_loop_thread();
  is_listening_ = true;
  channel_.enable_reading();
  socket_.listen();
}

void Acceptor::handle_read() {
  loop_->assert_in_loop_thread();
  InetAddress peer_addr;
  int connfd = socket_.accept(&peer_addr);
  if (connfd >= 0) {
    connection_callback_(connfd, peer_addr);
  }
}

void Acceptor::default_connection_callback(int connfd, const InetAddress &peer_addr) {
  std::cout << "accept new connection from " << peer_addr.to_string() << std::endl;
  std::string message("close connection\n");
  ::write(connfd, message.data(), message.size());
  ::close(connfd);
}
  
}