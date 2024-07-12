#include "Socket.h"

namespace webserver
{

InetAddress::InetAddress(uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  inet_pton(AF_INET, ip.data(), &addr_.sin_addr);
  addr_.sin_port = htons(port);
}

std::string InetAddress::to_string() const {
  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip));
  auto port = ntohs(addr_.sin_port);
  std::ostringstream stream;
  stream << ip << ":" << port;
  return stream.str();
}

Socket::Socket(int socket) : socket_(socket) {}
  
Socket::~Socket() {
  close(socket_);
}

int Socket::create_non_blocking_socket() {
  auto fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    LOG_SYSFATAL << "create_non_blocking_socket\n";
  }
  return fd;
}

void Socket::bind(InetAddress *addr) {
  auto ret = ::bind(socket_, addr->get_sockaddr(), InetAddress::socklen());
  if (ret < 0) {
     LOG_SYSFATAL << "Socket::bind\n";
  }
}

void Socket::listen() {
  int ret = ::listen(socket_, SOMAXCONN);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::listenOrDie";
  }
}

int Socket::accept(InetAddress *peer_addr) {
  socklen_t socklen = InetAddress::socklen();
  int connfd = accept4(socket_, peer_addr->get_sockaddr(), &socklen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int savedErrno = errno;
    LOG_SYSERR << "Socket::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}

void Socket::set_reuse_addr(bool flag) {
  int optval = flag ? 1 : 0;
  setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

}