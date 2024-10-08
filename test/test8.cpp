#include "TcpServer.h"
#include "EventLoop.h"
#include "Socket.h"

#include <iostream>
#include <stdio.h>

using namespace webserver;

void onConnection(const std::shared_ptr<TcpConnection>& conn) {
  if (conn->is_connected()) {
    std::cout << "onConnection(): new connection " << conn->name() << std::endl;
  }
  else {
    std::cout << "onConnection(): connection " << conn->name() << " is down\n";
  }
}

void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer *buf, Timestamp receive_time) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receive_time.toFormattedString().c_str());

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
  printf("main(): pid = %d\n", getpid());

  InetAddress listenAddr(9981);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.set_connection_callback(onConnection);
  server.set_message_callback(onMessage);
  server.start();

  loop.loop();
}

