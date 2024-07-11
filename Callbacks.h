#pragma once

#include <functional>
#include <memory>
#include <sys/types.h>

namespace webserver
{
class InetAddress;
class TcpConnection;

typedef std::function<void(int connfd, const InetAddress &peer_addr)> NewConnectionCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &)> ConnectionCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &, const char* data, ssize_t len)> MessageCallback;
  
}