#include "TcpClient.h"

namespace webserver
{

TcpClient::TcpClient(EventLoop *loop, const InetAddress &server_addr)
  : loop_(loop), retry_(false), connect_(true), next_conn_id_(1), 
    connector_(new Connector(loop, server_addr))
{
  connector_->set_new_connection_callback([this](int sockfd) { this->new_connection_callback(sockfd); });
  LOG_INFO << "TcpClient[" << this << "] - connector " << connector_.get() << "\n";
  // LOG_INFO << "TcpClient[" << "] - connector " << "\n";
}

TcpClient::~TcpClient() {
  LOG_INFO << "~TcpClient[" << this << "] - connector " << connector_.get() << "\n";
  // LOG_INFO << "~TcpClient[" << "] - connector " << "\n";
  std::shared_ptr<TcpConnection> tmp_conn;
  mutex_.lock();
  tmp_conn = tcp_connection_;
  mutex_.unlock();
  if (tmp_conn) {
    /**********************************************************************************/
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient[" << this << "] connecting to " << connector_->server_addr().to_string() << "\n";
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  mutex_.lock();
  if (tcp_connection_) {
    tcp_connection_->shutdown();
  }
  mutex_.unlock();
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::new_connection_callback(int sockfd) {
  loop_->assert_in_loop_thread();
  InetAddress peer_addr(Socket::get_peer_addr(sockfd));
  InetAddress local_addr(Socket::get_local_addr(sockfd));
  std::ostringstream conn_name_str;
  conn_name_str << ":" << peer_addr.to_string() << "#" << next_conn_id_;
  next_conn_id_++;
  auto tmp_connection = std::make_shared<TcpConnection>(loop_, sockfd, conn_name_str.str(), peer_addr, peer_addr);
  tmp_connection->set_connection_callback(connection_cb_);
  tmp_connection->set_message_callback(message_cb_);
  tmp_connection->set_write_complete_callback(write_complete_cb_);
  tmp_connection->set_close_callback([this](const std::shared_ptr<TcpConnection> conn) { this->remove_connection(conn); });
  mutex_.lock();
  tcp_connection_ = tmp_connection;
  mutex_.unlock();
  tmp_connection->initialize_connection();
}

void TcpClient::remove_connection(const std::shared_ptr<TcpConnection> &connection) {
  loop_->assert_in_loop_thread();
  assert(loop_ == connection->get_loop());
  assert(tcp_connection_ == connection);
  mutex_.lock();
  tcp_connection_.reset();
  mutex_.unlock();
  loop_->queue_in_loop([connection]() { connection->destroy_connection(); });
  if (retry_ && connect_) {
    LOG_INFO << "TcpClient[" << this << "] - reconnecting to " << connector_->server_addr().to_string() << "\n";
    connector_->restart();
  }
}
  
}