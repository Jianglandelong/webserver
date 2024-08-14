#include "EventLoop.h"
#include "Socket.h"
#include "TcpClient.h"

#include <utility>
#include <stdio.h>
#include <unistd.h>

std::string message = "Hello\n";

void onConnection(const std::shared_ptr<webserver::TcpConnection>& conn) {
  if (conn->is_connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peer_addr().to_string().c_str());
    conn->send(message);
  } else {
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

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
  webserver::EventLoop loop;
  webserver::InetAddress serverAddr("localhost", 9981);
  webserver::TcpClient client(&loop, serverAddr);

  client.set_connection_callback(onConnection);
  client.set_message_callback(onMessage);
  client.enable_retry();
  client.connect();
  loop.loop();
}

