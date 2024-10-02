#include "Connector.h"
#include <algorithm>

namespace webserver
{

const int Connector::max_retry_delay_ms;
const int Connector::init_retry_delay_ms;

Connector::Connector(EventLoop *loop, const InetAddress &server_addr) 
  : state_(State::Disconnected), is_start_(false), loop_(loop), server_addr_(server_addr), 
    retry_delay_ms_(init_retry_delay_ms)
{
  // LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
  // LOG_DEBUG << "dtor[" << this << "]";
  cancel_timer();
  assert(!channel_);
}

void Connector::start() {
  is_start_ = true;
  loop_->run_in_loop([this]() { this->start_in_loop(); });
}

void Connector::start_in_loop() {
  loop_->assert_in_loop_thread();
  assert(state_ == State::Disconnected);
  if (is_start_) {
    connect();
  } else {
    LOG_DEBUG << "Do not start\n";
  }
}

void Connector::connect() {
  auto sockfd = Socket::create_non_blocking_socket();
  auto ret = Socket::connect(sockfd, server_addr_);
  int saved_errno = (ret == 0) ? 0 : errno;
  switch (saved_errno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      init_connection(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << strerror(saved_errno) << "\n";
      ::close(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << strerror(saved_errno) << "\n";
      ::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}
  
void Connector::init_connection(int sockfd) {
  state_ = State::Connecting;
  channel_.reset(new Channel(loop_, sockfd));
  channel_->set_write_callback([this]() { this->handle_write(); });
  channel_->set_error_callback([this]() { this->handle_error(); });
  channel_->enable_writing();
}

void Connector::handle_write() {
  LOG_TRACE << "Connector::handleWrite " << state_ << "\n";

  if (state_ == State::Connecting) {
    int sockfd = remove_and_reset_channel();
    int err = Socket::get_socket_error(sockfd);
    if (err) {
      // LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " " << strerror_tl(err);
      LOG_WARN << "Connector::handle_write error" << strerror(err) << "\n";
      retry(sockfd);
    } else if (Socket::is_self_connect(sockfd)) {
      LOG_WARN << "Connector::handleWrite - Self connect" << "\n";
      retry(sockfd);
    } else {
      state_ = State::Connected;
      if (is_start_) {
        new_connection_cb_(sockfd);
      } else {
        ::close(sockfd);
      }
    }
  } else {
    // what happened?
    assert(state_ == State::Disconnected);
  }
}

void Connector::handle_error() {
  LOG_ERROR << "Connector::handleError\n";
  assert(state_ == State::Connecting);

  int sockfd = remove_and_reset_channel();
  int err = Socket::get_socket_error(sockfd);
  LOG_TRACE << "SO_ERROR = " << strerror(err) << "\n";
  retry(sockfd);
}

void Connector::retry(int sockfd) {
  ::close(sockfd);
  state_ = State::Disconnected;
  if (is_start_) {
    LOG_INFO << "Connector::retry - Retry connecting to "
             << server_addr_.to_string() << " in "
             << retry_delay_ms_ << " milliseconds.\n";
    retry_timer_ = loop_->run_after(retry_delay_ms_/1000.0,  [this]() { this->start_in_loop(); });
    retry_delay_ms_ = std::min(retry_delay_ms_ * 2, max_retry_delay_ms);
  } else {
    LOG_DEBUG << "do not connect\n";
  }
}

int Connector::remove_and_reset_channel() {
  channel_->disable_all();
  loop_->remove_channel(channel_.get());
  int fd = channel_->fd();
  loop_->queue_in_loop([this]() { this->reset_channel(); });
  return fd;
}

void Connector::reset_channel() {
  channel_.reset();
}

void Connector::stop() {
  is_start_ = false;
  cancel_timer();
  retry_timer_.reset();
}

void Connector::restart() {
  loop_->assert_in_loop_thread();
  state_ = State::Disconnected;
  retry_delay_ms_ = init_retry_delay_ms;
  is_start_ = true;
  start_in_loop();
}

void Connector::cancel_timer() {
  if (retry_timer_) {
    loop_->cancel_timer(std::move(retry_timer_));
  }
}

}