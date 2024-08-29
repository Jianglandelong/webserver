#pragma once

#include "HttpConnection.h"
#include "TcpServer.h"

namespace webserver
{

class HttpServer : public TcpServer {
public:
  HttpServer(EventLoop* loop, const InetAddress &addr);
  void new_connection_callback(int connfd, const InetAddress &peer_addr) override;
};
  
}