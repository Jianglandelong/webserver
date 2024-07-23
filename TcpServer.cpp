#include "TcpServer.h"

namespace webserver
{

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr) : 
  loop_(loop), addr_(addr), acceptor_(std::make_unique<Acceptor>(loop_, &addr_))
{
  acceptor_->set_connection_callback(
    [this](int connfd, const InetAddress &peer_addr) { this->new_connection_callback(connfd, peer_addr); }
  );
}

void TcpServer::start() {
  if (is_start_ || acceptor_->is_listening()) {
    return;
  }
  is_start_ = true;
  loop_->run_in_loop([this]() { this->acceptor_->listen(); });
}

void TcpServer::new_connection_callback(int connfd, const InetAddress &peer_addr) {
  loop_->assert_in_loop_thread();
  std::ostringstream name;
  name << addr_.to_string() << "#" << next_connection_id_;
  next_connection_id_++;
  LOG_INFO << "TcpServer new connection " << name.str() << " from " << peer_addr.to_string() << std::endl;
  // Initialize TcpConnection
  auto connection = std::make_shared<TcpConnection>(loop_, connfd, name.str(), addr_, peer_addr);
  connection_map_[name.str()] = connection;
  connection->set_connection_callback(connection_cb_);
  connection->set_message_callback(message_cb_);
  connection->set_close_callback([this](const std::shared_ptr<TcpConnection> &conn) { this->remove_connection(conn); });
  connection->set_write_complete_callback(write_complete_cb_);
  connection->initialize_connection();
}

void TcpServer::remove_connection(const std::shared_ptr<TcpConnection> &connection) {
  loop_->assert_in_loop_thread();
  LOG_INFO << "TcpServer::removeConnection: " << connection->name() << std::endl;
  connection_map_.erase(connection->name());
  loop_->queue_in_loop([connection]() { connection->destroy_connection(); });
}

}