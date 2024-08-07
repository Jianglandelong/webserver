#include "Connector.h"
#include "EventLoop.h"

#include <stdio.h>

webserver::EventLoop* g_loop;

void connectCallback(int sockfd)
{
  printf("#### connected.####\n");
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  webserver::EventLoop loop;
  g_loop = &loop;
  webserver::InetAddress addr("127.0.0.1", 9981);
  auto connector = std::make_shared<webserver::Connector>(&loop, addr);
  connector->set_new_connection_callback(connectCallback);
  connector->start();

  loop.loop();
}
