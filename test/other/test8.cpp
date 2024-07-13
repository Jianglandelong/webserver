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

void onMessage(const std::shared_ptr<TcpConnection>& conn, const char* data, ssize_t len) {
  std::cout << "onMessage(): received " << len << " bytes from connection " << conn->name() << std::endl;
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

