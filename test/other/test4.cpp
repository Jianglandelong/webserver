#include "EventLoop.h"

#include <stdio.h>
#include <unistd.h> // Add this line to include the header file for getpid

int cnt = 0;
webserver::EventLoop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  printf("now %s\n", webserver::Timestamp::now().toString().c_str());
}

void print(const char* msg)
{
  printf("msg %s %s\n", webserver::Timestamp::now().toString().c_str(), msg);
  if (++cnt == 20)
  {
    g_loop->quit();
  }
}

int main()
{
  printTid();
  webserver::EventLoop loop;
  g_loop = &loop;

  print("main");
  loop.run_after(1, std::bind(print, "once1"));
  loop.run_after(1.5, std::bind(print, "once1.5"));
  loop.run_after(2.5, std::bind(print, "once2.5"));
  loop.run_after(3.5, std::bind(print, "once3.5"));
  loop.run_every_interval(2, std::bind(print, "every2"));
  loop.run_every_interval(3, std::bind(print, "every3"));

  loop.loop();
  print("main loop exits");
  sleep(1);
}
