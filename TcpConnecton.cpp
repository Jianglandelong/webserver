#include "TcpConnection.h"

namespace webserver
{

TcpConnection::TcpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &server_addr, 
  const InetAddress &peer_addr) :
  loop_(loop), socket_(std::make_unique<Socket>(connfd)), name_(name),
  channel_(std::make_unique<Channel>(loop, connfd)), server_addr_(server_addr), peer_addr_(peer_addr)
{
  channel_->set_read_callback([this](Timestamp time) {this->handle_event(time); });
}
  
void TcpConnection::initialize_connection() {
  loop_->assert_in_loop_thread();
  assert(!is_connected());
  state_ = State::connected;
  channel_->enable_reading();
  connection_cb_(shared_from_this());
}

void TcpConnection::destroy_connection() {
  loop_->assert_in_loop_thread();
  channel_->disable_events();
  assert(state_ == State::connected);
  state_ = State::disconnected;
  connection_cb_(shared_from_this());
  loop_->remove_channel(channel_.get());
}

void TcpConnection::handle_event(Timestamp receive_time) {
  int tmp_errno = 0;
  auto n = input_buffer_.readFd(channel_->fd(), &tmp_errno);
  if (n > 0) {
    message_cb_(shared_from_this(), &input_buffer_, receive_time);
  } else if (n == 0) {
    handle_close();
  } else {
    errno = tmp_errno;
    handle_error();
  }
}

void TcpConnection::handle_close() {
  loop_->assert_in_loop_thread();
  LOG_TRACE << "TcpConnection::handleClose state = " << state_ << std::endl;
  channel_->disable_events();
  close_cb_(shared_from_this());
}

void TcpConnection::handle_error() {
  int err = Socket::get_socket_error(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << std::strerror(err);
}
  
}