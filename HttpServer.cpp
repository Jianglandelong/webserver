#include "HttpServer.h"

namespace webserver
{

HttpServer::HttpServer(EventLoop* loop, const InetAddress &addr) : TcpServer(loop, addr) {}

void HttpServer::new_connection_callback(int connfd, const InetAddress &peer_addr) {
  loop_->assert_in_loop_thread();
  std::ostringstream name;
  name << addr_.to_string() << "#" << next_connection_id_;
  next_connection_id_++;
  LOG_INFO << "HttpServer new connection " << name.str() << " from " << peer_addr.to_string() << std::endl;
  // Initialize TcpConnection
  auto new_loop = thread_pool_->get_next_loop();
  auto connection = std::make_shared<HttpConnection>(new_loop, connfd, name.str(), addr_, peer_addr);
  connection_map_[name.str()] = connection;
  // connection->set_connection_callback(connection_cb_);
  // connection->set_message_callback(message_cb_);
  connection->set_close_callback([this](const std::shared_ptr<TcpConnection> &conn) { this->remove_connection(conn); });
  // connection->set_write_complete_callback(write_complete_cb_);
  new_loop->run_in_loop([connection]() { connection->initialize_connection(); });
}
  
}