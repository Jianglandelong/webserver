#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());

  webserver::EventLoopThread loopThread;
  webserver::EventLoop* loop = loopThread.start_loop();
  loop->run_in_loop(runInThread);
  sleep(1);
  loop->run_after(2, runInThread);
  sleep(3);
  loop->quit();

  printf("exit main().\n");
}
