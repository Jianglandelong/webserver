#include "Channel.h"
#include "EventLoop.h"

#include <cstdio>
#include <sys/timerfd.h>

webserver::EventLoop* g_loop;

void timeout() {
  printf("Timeout\n");
  g_loop->quit();
}

int main() {
  webserver::EventLoop loop;
  g_loop = &loop;

  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  webserver::Channel channel(&loop, timer_fd);
  channel.set_read_callback(timeout);
  channel.enable_reading();

  struct itimerspec howlong;
  // bzero(&howlong, sizeof(howlong));
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timer_fd, 0, &howlong, nullptr);

  loop.loop();
  close(timer_fd);
  
  return 0;
}