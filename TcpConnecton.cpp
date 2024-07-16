#include "TcpConnection.h"

namespace webserver
{

TcpConnection::TcpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &server_addr, 
  const InetAddress &peer_addr) :
  loop_(loop), socket_(std::make_unique<Socket>(connfd)), name_(name),
  channel_(std::make_unique<Channel>(loop, connfd)), server_addr_(server_addr), peer_addr_(peer_addr)
{
  channel_->set_read_callback([this](Timestamp time) {this->handle_read(time); });
  channel_->set_write_callback([this]() { this->handle_write(); });
  channel_->set_close_callback([this]() { this->handle_close(); });
  channel_->set_error_callback([this]() { this->handle_error(); });
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
  assert(state_ == State::connected || state_ == State::disconnecting);
  state_ = State::disconnected;
  connection_cb_(shared_from_this());
  loop_->remove_channel(channel_.get());
}

void TcpConnection::handle_read(Timestamp receive_time) {
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
  LOG_TRACE << "TcpConnection::handle_close state = " << state_ << std::endl;
  channel_->disable_events();
  close_cb_(shared_from_this());
}

void TcpConnection::handle_error() {
  int err = Socket::get_socket_error(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << std::strerror(err);
}

void TcpConnection::shutdown() {
  if (state_ == State::connected) {
    state_ = State::disconnecting;
    loop_->run_in_loop([this]() { this->shutdown_in_loop(); });
  }
}

void TcpConnection::shutdown_in_loop() {
  loop_->assert_in_loop_thread();
  if (!channel_->is_writing()) {
    socket_->shutdown_write();
  }
}

void TcpConnection::send(const std::string &message) {
  if (state_ == State::connected) {
    loop_->run_in_loop([this, msg = std::move(message)]() { this->send_in_loop(msg); });
  }
}

void TcpConnection::send_in_loop(const std::string &message) {
  loop_->assert_in_loop_thread();
  ssize_t num;
  if (!channel_->is_writing() && output_buffer_.readableBytes() == 0) {
    num = ::write(socket_->fd(), message.c_str(), message.size());
    if (num < 0 && num > message.size()) {
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::send_in_loop() error\n";
      }
    }
  }
  assert(num >= 0);
  if (num < message.size()) {
    LOG_TRACE << "more data to be sent\n";
    output_buffer_.append(message.data() + num, message.size() - num);
    if (!channel_->is_writing()) {
      channel_->enable_writing();
    }
  }
}

void TcpConnection::handle_write() {
  loop_->assert_in_loop_thread();
  if (channel_->is_writing()) {
    auto num = ::write(channel_->fd(), output_buffer_.peek(), output_buffer_.readableBytes());
    if (num >= 0) {
      output_buffer_.retrieve(num);
      if (output_buffer_.readableBytes() == 0) {
        channel_->disable_writing();
        if (state_ == State::disconnecting) {
          shutdown_in_loop();
        }
      } else {
        LOG_TRACE << "more data to be sent\n";
      }
    } else {
      LOG_SYSERR << "TcpConnection::handle_write() error\n";
    }
  } else {
    LOG_TRACE << "Connection is down, no more writing\n";
  }
}
  
}