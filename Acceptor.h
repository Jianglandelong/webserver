#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"
#include "Socket.h"

namespace webserver
{

class Acceptor {
public:
  typedef std::function<void(int connfd, const InetAddress &peer_addr)> ConnectionCallback;

  Acceptor(EventLoop *loop, InetAddress *addr);
  void listen();
  void handle_read();
  void set_connection_callback(const ConnectionCallback &cb) { connection_callback_ = cb; }
  static void default_connection_callback(int connfd, const InetAddress &peer_addr);

private:
  bool is_listening_{false};
  EventLoop *loop_;
  Socket socket_;
  Channel channel_;
  ConnectionCallback connection_callback_;
};
  
}