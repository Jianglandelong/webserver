#pragma once

#include "Timer.h"

#include <functional>
#include <memory>
#include <sys/types.h>

namespace webserver
{
class InetAddress;
class TcpConnection;
class Buffer;

typedef std::function<void(int connfd, const InetAddress &peer_addr)> NewConnectionCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &)> ConnectionCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &, Buffer *buf, Timestamp time)> MessageCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &)> CloseCallback;
typedef std::function<void(const std::shared_ptr<TcpConnection> &)> WriteCompleteCallback;
  
}