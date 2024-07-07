#include "Logging.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace webserver
{

class InetAddress {
public:
  explicit InetAddress(uint16_t port = 0);
  InetAddress(const std::string& ip, uint16_t port);  // ip: 1.2.3.4
  InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  sockaddr_in socket_in() { return addr_; }
  sockaddr* get_sockaddr() { return reinterpret_cast<struct sockaddr*>(&addr_); }
  static socklen_t socklen() { return sizeof(struct sockaddr_in); }
  std::string to_string() const;
  
private:
  sockaddr_in addr_;
};

class Socket {
public:
  Socket(int socket);
  ~Socket();
  static int create_non_blocking_socket();
  int fd() { return socket_; }
  void bind(InetAddress *addr);
  void listen();
  int accept(InetAddress *peer_addr);
  void set_reuse_addr(bool flag);

private:
  int socket_;
};
  
}