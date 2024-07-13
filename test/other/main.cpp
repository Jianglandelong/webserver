#include "EventLoop.h"

#include <iostream>
#include <thread>

using namespace webserver;

EventLoop* g_loop;

void print() {
  std::cout << "print\n";
}

void thread_func() {
  g_loop->run_after(2, print);
}

int main() {
  EventLoop loop;
  g_loop = &loop;
  auto trd = std::thread(thread_func);
  loop.loop();
  return 0;
}