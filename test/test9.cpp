#include "TcpServer.h"
#include "EventLoop.h"
#include "Socket.h"
#include <stdio.h>

void onConnection(const std::shared_ptr<webserver::TcpConnection>& conn)
{
  if (conn->is_connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peer_addr().to_string());
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const std::shared_ptr<webserver::TcpConnection>& conn,
               webserver::Buffer* buf,
               webserver::Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  conn->send(buf->retrieveAsString());
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  webserver::InetAddress listenAddr(9981);
  webserver::EventLoop loop;

  webserver::TcpServer server(&loop, listenAddr);
  server.set_connection_callback(onConnection);
  server.set_message_callback(onMessage);
  server.start();

  loop.loop();
}
