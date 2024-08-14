#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Poller.h"

#include <algorithm>

namespace webserver
{

Poller::Poller(EventLoop* loop) : loop_(loop) {}

Poller::~Poller() {}

Timestamp Poller::poll(std::vector<Channel*> &active_channels, int timeout) {
  int num_revents = ::poll(poll_fds_.data(), poll_fds_.size(), timeout);
  auto now = Timestamp::now();
  if (num_revents > 0) {
    // LOG_TRACE << num_revents << " events happen";
    LOG << num_revents << " events happen\n";
    fill_active_channels(num_revents, active_channels);
  } else if (num_revents == 0) {
    // LOG_TRACE << "nothing happens\n";
  } else {
    // LOG_SYSERR << "Poller:poll()";
    LOG << "Poller:poll()\n";
  }
  return now;
}

void Poller::fill_active_channels(int num_revents, std::vector<Channel*> &active_channels) {
  for (const auto &poll_fd : poll_fds_) {
    if (num_revents > 0 && poll_fd.revents > 0) {
      num_revents--;
      auto iter = channel_map_.find(poll_fd.fd);
      assert(iter != channel_map_.end());
      auto &channel = iter->second;
      assert(channel->fd() == poll_fd.fd);
      channel->set_revents(poll_fd.revents);
      active_channels.push_back(channel);
    }
  }
}

void Poller::update_channel(Channel* new_channel) {
  assert_in_loop_thread();
  auto iter = channel_map_.find(new_channel->fd());
  // add new channel and new pollfd
  if (new_channel->index_in_pollfds() < 0) {
    assert(iter == channel_map_.end());
    new_channel->set_index(poll_fds_.size());
    channel_map_[new_channel->fd()] = new_channel;
    pollfd pfd;
    pfd.fd = new_channel->fd();
    pfd.events = new_channel->events();
    pfd.revents = 0;
    poll_fds_.push_back(pfd);
  } else {  // update channel
    assert(iter != channel_map_.end());
    assert(new_channel->index_in_pollfds() < poll_fds_.size());
    auto &pfd = poll_fds_[new_channel->index_in_pollfds()];
    assert(pfd.fd == new_channel->fd() || pfd.fd == -new_channel->fd() - 1);  // -1 means the fd is ignored
    pfd.events = new_channel->events();
    pfd.revents = 0;
    if (new_channel->is_none_event()) {
      pfd.fd = -new_channel->fd() - 1;
    }
  }
}

void Poller::remove_channel(Channel *channel) {
  assert_in_loop_thread();
  // LOG_TRACE << "fd = " << channel->fd();
  auto iter = channel_map_.find(channel->fd());
  auto removed_index = channel->index_in_pollfds();
  assert(iter != channel_map_.end());
  assert(iter->second == channel);
  assert(channel->is_none_event());
  assert(0 <= removed_index && removed_index < poll_fds_.size());
  channel_map_.erase(iter);
  if (removed_index == poll_fds_.size() - 1) {
    poll_fds_.pop_back();
  } else { // swap the removed pollfd with the last pollfd in poll_fds_, then pop back
    auto swap_fd = poll_fds_.back().fd;
    if (swap_fd < 0) {
      swap_fd = -swap_fd - 1;
    }
    std::swap(poll_fds_[removed_index], poll_fds_.back());
    poll_fds_.pop_back();
    channel_map_[swap_fd]->set_index(removed_index);
  }
}

EPoller::EPoller(EventLoop *loop) : loop_(loop) {
  epfd_ = ::epoll_create(EPOLL_CLOEXEC);
  epoll_events_.resize(init_epoll_events_size_);
}

EPoller::~EPoller() {
  ::close(epfd_);
}

Timestamp EPoller::poll(std::vector<Channel*> &active_channels, int timeout) {
  int num_events = ::epoll_wait(epfd_, epoll_events_.data(), epoll_events_.size(), timeout);
  auto now = Timestamp::now();
  if (num_events > 0) {
    // LOG_TRACE << num_revents << " events happen";
    LOG << num_events << " events happen\n";
    if (num_events == epoll_events_.size()) {
      epoll_events_.resize(epoll_events_.size() * 2);
    }
    fill_active_channels(num_events, active_channels);
  } else if (num_events == 0) {
    // LOG_TRACE << "nothing happens\n";
  } else {
    // LOG_SYSERR << "Poller:poll()";
    LOG << "EPoller:poll() " << strerror(errno) << std::endl;
  }
  return now;
}

void EPoller::fill_active_channels(int num_events, std::vector<Channel*> &active_channels) {
  for (int i = 0; i < num_events; i++) {
    auto new_channel = static_cast<Channel*>(epoll_events_[i].data.ptr);
    new_channel->set_revents(epoll_events_[i].events);
    active_channels.push_back(new_channel);
  }
}

void EPoller::update_channel(Channel *new_channel) {
  loop_->assert_in_loop_thread();
  int fd = new_channel->fd();
  auto iter = channel_map_.find(fd);
  if (iter != channel_map_.end()) {
    assert(iter->second == new_channel);
    epoll_event new_event;
    new_event.data.ptr = static_cast<void*>(new_channel);
    new_event.events = new_channel->events();
    if (new_channel->is_none_event()) {
      ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &new_event);
    } else {
      ::epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &new_event);
    }
  } else {
    channel_map_[fd] = new_channel;
    epoll_event new_event;
    new_event.data.ptr = static_cast<void*>(new_channel);
    new_event.events = new_channel->events();
    ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &new_event);
  }
}

void EPoller::remove_channel(Channel *channel) {
  loop_->assert_in_loop_thread();
  auto fd = channel->fd();
  auto iter = channel_map_.find(fd);
  assert(iter != channel_map_.end());
  channel_map_.erase(iter);
  ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
}

}