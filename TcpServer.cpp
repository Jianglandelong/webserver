#include "TcpServer.h"

namespace webserver
{

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr) : 
  loop_(loop), addr_(addr), acceptor_(std::make_unique<Acceptor>(loop_, &addr_)), 
  thread_pool_(std::make_unique<EventLoopThreadPool>(loop_))
{
  acceptor_->set_connection_callback(
    [this](int connfd, const InetAddress &peer_addr) { this->new_connection_callback(connfd, peer_addr); }
  );
}

void TcpServer::start() {
  if (!is_start_) {
    thread_pool_->start();
    is_start_ = true;
  }
  if (!acceptor_->is_listening()) {
    loop_->run_in_loop([this]() { this->acceptor_->listen(); });
  }
}

void TcpServer::new_connection_callback(int connfd, const InetAddress &peer_addr) {
  loop_->assert_in_loop_thread();
  std::ostringstream name;
  name << addr_.to_string() << "#" << next_connection_id_;
  next_connection_id_++;
  LOG_INFO << "TcpServer new connection " << name.str() << " from " << peer_addr.to_string() << "\n";
  // Initialize TcpConnection
  auto new_loop = thread_pool_->get_next_loop();
  auto connection = std::make_shared<TcpConnection>(new_loop, connfd, name.str(), addr_, peer_addr);
  connection_map_[name.str()] = connection;
  connection->set_connection_callback(connection_cb_);
  connection->set_message_callback(message_cb_);
  connection->set_close_callback([this](const std::shared_ptr<TcpConnection> &conn) { this->remove_connection(conn); });
  connection->set_write_complete_callback(write_complete_cb_);
  new_loop->run_in_loop([connection]() { connection->initialize_connection(); });
}

void TcpServer::remove_connection(const std::shared_ptr<TcpConnection> &connection) {
  loop_->run_in_loop([this, connection]() { this->remove_connection_in_loop(connection); });
}

void TcpServer::remove_connection_in_loop(const std::shared_ptr<TcpConnection> &connection) {
  loop_->assert_in_loop_thread();
  LOG_INFO << "TcpServer::removeConnection: " << connection->name() << "\n";
  connection_map_.erase(connection->name());
  auto connection_loop = connection->get_loop();
  connection_loop->queue_in_loop([connection]() { connection->destroy_connection(); });
}

void TcpServer::set_thread_num(int num_threads) {
  assert(num_threads >= 0);
  thread_pool_->set_thread_num(num_threads);
}

}