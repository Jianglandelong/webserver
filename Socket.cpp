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

int Socket::create_socket() {
  auto fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    LOG_SYSFATAL << "Socket::create_socket() error\n";
  }
  return fd;
}

int Socket::create_non_blocking_socket() {
  auto fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    LOG_SYSFATAL << "Socket::create_non_blocking_socket() error\n";
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

int Socket::connect(int sockfd, InetAddress &server_addr) {
  return ::connect(sockfd, server_addr.get_sockaddr(), InetAddress::socklen());
}

void Socket::set_reuse_addr(bool flag) {
  int optval = flag ? 1 : 0;
  setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::shutdown_write() {
  if (::shutdown(socket_, SHUT_WR) < 0) {
    LOG_SYSERR << "Socket::shutdown_write()" << std::endl;
  }
}

void Socket::set_no_delay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd(), IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval));
}

bool Socket::is_self_connect(int sockfd) {
  struct sockaddr_in localaddr = get_local_addr(sockfd);
  struct sockaddr_in peeraddr = get_peer_addr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

sockaddr_in Socket::get_local_addr(int sockfd)
{
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, reinterpret_cast<sockaddr*>(&localaddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localaddr;
}

sockaddr_in Socket::get_peer_addr(int sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, reinterpret_cast<sockaddr*>(&peeraddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getPeerAddr";
  }
  return peeraddr;
}

void Socket::set_non_block(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(sockfd, F_SETFL, flags) == -1) {
    LOG_ERROR << "Socket::set_non_block() error\n";
  }
}

}