#pragma once

#include "TcpConnection.h"

#include <cstdarg>
#include <optional>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>

namespace webserver
{

class HttpBuffer {
public:
  HttpBuffer() : input_index_(0), output_index_(0) {}
  HttpBuffer(const int buffer_size) : input_index_(0), output_index_(0) { buf_.reserve(buffer_size); }
  ssize_t read_fd(int fd, int* savedErrno);
  size_t writable_bytes() { return buf_.size() - input_index_; }
  size_t readable_bytes() { return input_index_ - output_index_; }
  std::optional<std::string> get_next_line();
  std::string get_remain();
  void append(const char *data, size_t len);
  int input(const char *format, ...);
  char* begin() { return buf_.data(); }
  const char *peek() { return begin() + output_index_; }
  void retrieve(size_t len) {
    assert(len <= readable_bytes());
    output_index_ += len;
  }
  void retrieve_all() {
    input_index_ = 0;
    output_index_ = 0;
  }
  void reset(const int buffer_size) {
    buf_.clear();
    buf_.reserve(buffer_size);
    retrieve_all();
  }

  static const int Init_input_buffer_size = 2048;
  static const int Init_output_buffer_size = 1024;
  
private:
  std::string buf_;
  int input_index_;
  int output_index_;
};

class HttpConnection : public TcpConnection {
public:
  enum Method {GET, POST};
  enum CheckState {Check_request_line, Check_header, Check_content};
  enum HttpCode {
    No_request,  // 请求不完整
    Get_request,  // 获得完整请求
    Bad_request,  // 请求有语法错误
    No_resource, 
    Forbidden_request, 
    File_request, 
    Internal_error, 
    Closed_connection
  };
  // enum LineStatus {OK, BAD, OPEN};  // {完整读取一行，语法错误，读取的行不完整}

  HttpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &loacl_addr, 
    const InetAddress &peer_addr);

protected:
  Method method_;
  CheckState check_state_;
  std::string file_name_;
  std::string url_;
  std::string version_;
  std::string host_;
  std::string text_;
  char* file_addr_;
  struct stat file_stat_;
  bool keep_alive_;
  int content_len_;
  HttpBuffer input_buffer_;
  HttpBuffer output_buffer_;

  void init_http_connection();
  void connection_callback(const std::shared_ptr<webserver::TcpConnection>& conn);
  void handle_read(Timestamp receive_time) override;
  void handle_write() override;
  auto process_input() -> HttpCode;
  bool prepare_output(HttpCode read_flag);
  auto parse_request_line(std::string &text) -> HttpCode;
  auto parse_headers(std::string &text) -> HttpCode;
  auto parse_content() -> HttpCode;
  auto do_request() -> HttpCode;
  // int add_response(const char *format, ...);
  void add_status_line(int status, const std::string &title);
  void add_headers(int content_lenth);
  bool add_content(const std::string &content);
  void add_file() { output_buffer_.append(file_addr_, file_stat_.st_size); }
  void unmap();
};
  
}