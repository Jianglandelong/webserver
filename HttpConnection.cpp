#include "HttpConnection.h"

#include <sys/uio.h>

namespace webserver
{

//定义http响应的一些状态信息
const std::string ok_200_title = "OK";
const std::string error_400_title = "Bad Request";
const std::string error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const std::string error_403_title = "Forbidden";
const std::string error_403_form = "You do not have permission to get file form this server.\n";
const std::string error_404_title = "Not Found";
const std::string error_404_form = "The requested file was not found on this server.\n";
const std::string error_500_title = "Internal Error";
const std::string error_500_form = "There was an unusual problem serving the request file.\n";

ssize_t HttpBuffer::read_fd(int fd, int* savedErrno)
{
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writable_bytes();
  vec[0].iov_base = begin() + input_index_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    input_index_ += n;
  } else {
    input_index_ = buf_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

void HttpBuffer::append(const char *data, size_t len) {
  if (writable_bytes() < len) {
    buf_.resize(buf_.size() + len - writable_bytes());
  }
  std::copy(data, data + len, begin() + input_index_);
  input_index_ += len;
}

std::optional<std::string> HttpBuffer::get_next_line() {
  auto pos = buf_.find("\r\n", output_index_);
  if (pos < 0) {
    return std::nullopt;
  }
  auto res = buf_.substr(output_index_, pos - output_index_);
  output_index_ = pos + 2;
  return res;
}

std::string HttpBuffer::get_remain() {
  auto tmp_output_index = output_index_;
  output_index_ = input_index_;
  return buf_.substr(tmp_output_index);
}

int HttpBuffer::input(const char *format, ...) {
  va_list arg_list;
  va_list arg_list_copy;
  va_start(arg_list, format);
  va_copy(arg_list_copy, arg_list);
  int len = vsnprintf(nullptr, 0, format, arg_list_copy);
  if (len >= writable_bytes()) {
    buf_.resize(buf_.size() + len - writable_bytes() + 1);
  }
  int len_2 = vsnprintf(begin() + input_index_, writable_bytes(), format, arg_list); 
  assert(len == len_2);
  input_index_ += len;
  va_end(arg_list_copy);
  va_end(arg_list);
  return len;
}

HttpConnection::HttpConnection(EventLoop *loop, int connfd, const std::string &name, const InetAddress &loacl_addr, 
    const InetAddress &peer_addr)
    : TcpConnection(loop, connfd, name, loacl_addr, peer_addr)
{
  init_http_connection();
}

void HttpConnection::init_http_connection() {
  method_ = Method::GET;
  check_state_ = CheckState::Check_request_line;
  file_name_.clear();
  url_.clear();
  version_.clear();
  keep_alive_ = false;
  content_len_ = 0;
  input_buffer_.reset(HttpBuffer::Init_input_buffer_size);
  output_buffer_.reset(HttpBuffer::Init_output_buffer_size);
  set_connection_callback([this](const std::shared_ptr<webserver::TcpConnection>& conn)
    { this->connection_callback(conn); });
}

void HttpConnection::connection_callback(const std::shared_ptr<webserver::TcpConnection>& conn) {
  if (conn->is_connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peer_addr().to_string().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void HttpConnection::handle_read(Timestamp receive_time) {
  int tmp_errno = 0;
  auto n = input_buffer_.read_fd(channel_->fd(), &tmp_errno);
  if (n > 0) {
    auto read_flag = process_input();
    if (read_flag == HttpCode::No_request) {
      return;
    }
    auto write_flag = prepare_output(read_flag);
    if (!write_flag) {
      LOG_INFO << "prepare_output() fail" << "\n";
      handle_close();
    }
    channel_->enable_writing();
  } else if (n == 0) {
    handle_close();
  } else {
    errno = tmp_errno;
    handle_error();
  }
}

auto HttpConnection::process_input() -> HttpConnection::HttpCode {
  auto parse_res = HttpCode::No_request;
  std::optional<std::string> text;
  bool content_parsed = false;

  // POST 报文的请求数据不以 \r\n 结尾，只使用 parse_line() 判断会提前跳出循环
  // while ((check_state_ == CheckState::Check_content && !content_parsed) || 
  //   (text = input_buffer_.get_next_line()).has_value())
  while ((text = input_buffer_.get_next_line()).has_value() || 
    (check_state_ == CheckState::Check_content && !content_parsed))
  {
    LOG_INFO << text.value_or("") << "\n";
    switch (check_state_) {
      case CheckState::Check_request_line: {
        if (parse_request_line(text.value()) == HttpCode::Bad_request) {
          return HttpCode::Bad_request;
        }
        break;
      }
      case CheckState::Check_header: {
        parse_res = parse_headers(text.value());
        if (parse_res == HttpCode::Bad_request)
          return HttpCode::Bad_request;
        if (parse_res == HttpCode::Get_request) {
          return do_request();
        }
        break;
      }
      case CheckState::Check_content: {
        parse_res = parse_content();
        content_parsed = true;
        if (parse_res == HttpCode::Get_request)
          return do_request();
        break;
      }
      default:
        return HttpCode::Internal_error;
        break;
    }
  }
  return HttpCode::No_request;
}

auto HttpConnection::parse_request_line(std::string &text) -> HttpConnection::HttpCode {
  int interval_start = 0;
  int interval_end = text.find_first_of(" \t", interval_start);
  if (interval_end == -1) {
    return HttpCode::Bad_request;
  }
  auto method = std::string_view(text.data() + interval_start, interval_end - interval_start);
  if (method == "GET") {
    method_ = Method::GET;
  } else if (method == "POST") {
    method_ = Method::POST;
  } else {
    return HttpCode::Bad_request;
  }
  interval_start = text.find_first_not_of(" \t", interval_end + 1);
  interval_end = text.find_first_of(" \t", interval_start);
  if (interval_start == -1 || interval_end == -1) {
    return HttpCode::Bad_request;
  }
  url_ = text.substr(interval_start, interval_end - interval_start);
  interval_start = text.find_first_not_of(" \t", interval_end + 1);
  if (interval_start == -1) {
    return HttpCode::Bad_request;
  }
  version_ = text.substr(interval_start);
  if (version_ != "HTTP/1.1" && version_ != "HTTP/1.0") {
    return HttpCode::Bad_request;
  }
  check_state_ = CheckState::Check_header;
  return HttpCode::No_request;
}

auto HttpConnection::parse_headers(std::string &text) -> HttpConnection::HttpCode {
  if (text.empty()) {   // 请求头结束后的空行
    if (content_len_ > 0) {
      check_state_ = CheckState::Check_content;
      return HttpCode::No_request;
    }
    return HttpCode::Get_request;
  }
  char* c_text = text.data();
  if (strncasecmp(c_text, "Connection:", 11) == 0) {
    c_text += 11;
    c_text += strspn(c_text, " \t");
    if (strcasecmp(c_text, "keep-alive") == 0) {
        keep_alive_ = true;
    }
  } else if (strncasecmp(c_text, "Content-length:", 15) == 0) {
      c_text += 15;
      c_text += strspn(c_text, " \t");
      content_len_ = atol(c_text);
  } else if (strncasecmp(c_text, "Host:", 5) == 0) {
      c_text += 5;
      c_text += strspn(c_text, " \t");
      host_ = std::string(c_text);
  }
  return HttpCode::No_request;
}

auto HttpConnection::parse_content() -> HttpConnection::HttpCode {
  if (input_buffer_.readable_bytes() >= content_len_) {
    text_ = input_buffer_.get_remain();
    LOG_INFO << text_ << "\n";
    return HttpCode::Get_request;
  }
  return HttpCode::No_request;
}

auto HttpConnection::do_request() -> HttpConnection::HttpCode {
  if (url_ == "/") {
    return HttpCode::File_request;
  }
  file_name_ = "asset/hello.html";
  if (stat(file_name_.c_str(), &file_stat_) < 0) {
    LOG_INFO << "stat file error: " << strerror(errno) << "\n";
    return HttpCode::No_resource;
  }
  if (!(file_stat_.st_mode & S_IROTH)) {
    return HttpCode::Forbidden_request;
  }
  if (S_ISDIR(file_stat_.st_mode)) {
    return HttpCode::Bad_request;
  }
  auto fd = ::open(file_name_.c_str(), O_RDONLY);
  file_addr_ = (char*)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  ::close(fd);
  return HttpCode::File_request;
}

// int HttpConnection::add_response(const char *format, ...) {
//   va_list arg_list;
//   va_start(arg_list, format);
//   auto len = output_buffer_.input(format, arg_list);
//   va_end(arg_list);
//   return len;
// }

void HttpConnection::add_status_line(int status, const std::string &title) {
  output_buffer_.input("%s %d %s\r\n", "HTTP/1.1", status, title.c_str());
}

void HttpConnection::add_headers(int content_lenth) {
  output_buffer_.input("Content-Length: %d\r\n", content_lenth);
  output_buffer_.input("Connection: %s\r\n", (keep_alive_ == true) ? "keep-alive" : "close");
  output_buffer_.input("%s", "\r\n");
}

bool HttpConnection::add_content(const std::string &content) {
  auto len = output_buffer_.input("%s", content.c_str());
  return len == content.size();
}

bool HttpConnection::prepare_output(HttpCode read_flag) {
  switch (read_flag) {
    case HttpCode::Internal_error: {
      add_status_line(500, error_500_title);
      add_headers(error_500_form.size());
      if (!add_content(error_500_form))
        return false;
      break;
    }
    case HttpCode::Bad_request: {
      add_status_line(404, error_404_title);
      add_headers(error_404_form.size());
      if (!add_content(error_404_form))
        return false;
      break;
    }
    case HttpCode::Forbidden_request: {
      add_status_line(403, error_403_title);
      add_headers(error_403_form.size());
      if (!add_content(error_403_form))
        return false;
      break;
    }
    case HttpCode::File_request: {
      add_status_line(200, ok_200_title);
      if (file_stat_.st_size == 0) {
        const std::string ok_string = "<html><body></body></html>";
        add_headers(ok_string.size());
        if (!add_content(ok_string)) {
          return false;
        }
        return true;
      }
      add_headers(file_stat_.st_size);
      add_file();
      return true;
    }
    default:
      return false;
  }
}

void HttpConnection::handle_write() {
  loop_->assert_in_loop_thread();
  if (channel_->is_writing()) {
    auto num = ::write(channel_->fd(), output_buffer_.peek(), output_buffer_.readable_bytes());
    if (num > 0) {
      output_buffer_.retrieve(num);
      if (output_buffer_.readable_bytes() == 0) {
        channel_->disable_writing();
        unmap();
        if (keep_alive_) {
          init_http_connection();
        } else {
          handle_close();
        }
      }
    } else {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        unmap();
      }
      LOG_SYSERR << "TcpConnection::handle_write() error " << strerror(errno) << "\n";
    }
  } else {
    LOG_TRACE << "Connection is down, no more writing\n";
  }
}

void HttpConnection::unmap() {
  if (file_addr_) {
    munmap(file_addr_, file_stat_.st_size);
    file_addr_ = nullptr;
  }
}
  
}