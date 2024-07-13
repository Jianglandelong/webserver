#include "Acceptor.h"
#include "EventLoop.h"
#include <stdio.h>

void newConnection(int sockfd, const webserver::InetAddress& peer_addr)
{
  std::cout << "accept new connection from " << peer_addr.to_string() << std::endl;
  ::write(sockfd, "How are you?\n", 13);
  ::close(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  webserver::InetAddress listenAddr(9981);
  webserver::EventLoop loop;

  webserver::Acceptor acceptor(&loop, &listenAddr);
  acceptor.set_connection_callback(newConnection);
  acceptor.listen();

  loop.loop();
}
