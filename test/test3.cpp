#include "Channel.h"
#include "EventLoop.h"

#include <cstring>
#include <stdio.h>
#include <sys/timerfd.h>

webserver::EventLoop* g_loop;

void timeout(webserver::Timestamp receiveTime)
{
  printf("%s Timeout!\n", receiveTime.toFormattedString().c_str());
  g_loop->quit();
}

int main()
{
  printf("%s started\n", webserver::Timestamp::now().toFormattedString().c_str());
  webserver::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  webserver::Channel channel(&loop, timerfd);
  channel.set_read_callback(timeout);
  channel.enable_reading();

  struct itimerspec howlong;
  memset(&howlong, 0, sizeof(howlong));
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  ::close(timerfd);
}
