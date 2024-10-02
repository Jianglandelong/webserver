#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <cstring>

int sendHttpRequestKeepAlive(const std::string& url, std::string &response) {
  static int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(9981);
  if (inet_pton(AF_INET, url.c_str(), &(serverAddress.sin_addr)) <= 0) {
    std::cerr << "Invalid address/Address not supported" << std::endl;
    close(sock);
    return -1;
  }
  static bool is_first_call = true;
  if (is_first_call) {
    is_first_call = false;
    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
      std::cerr << "Connection failed" << std::endl;
      close(sock);
      return -1;
    }
  }
  // Send HTTP GET request
  std::string request = "GET / HTTP/1.1\r\n";
  request += "Host: " + url + "\r\n";
  request += "Connection: keep-alive\r\n";
  request += "\r\n";
  if (send(sock, request.c_str(), request.length(), 0) < 0) {
    std::cerr << "Failed to send request" << std::endl;
    close(sock);
    return -1;
  }
  // Receive and print response
  char buffer[4096];
  memset(buffer, 0, sizeof(buffer));
  int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
  response += buffer;
  return sock;
}

void sendHttpRequest(const std::string& url, std::string &response) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(9981);
  if (inet_pton(AF_INET, url.c_str(), &(serverAddress.sin_addr)) <= 0) {
    std::cerr << "Invalid address/Address not supported" << std::endl;
    close(sock);
    return;
  }
  if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Connection failed" << std::endl;
    close(sock);
    return;
  }
  std::string request = "GET / HTTP/1.1\r\n";
  request += "Host: " + url + "\r\n";
  request += "Connection: close\r\n";
  request += "\r\n";
  if (send(sock, request.c_str(), request.length(), 0) < 0) {
    std::cerr << "Failed to send request" << std::endl;
    close(sock);
    return;
  }
  char buffer[4096];
  memset(buffer, 0, sizeof(buffer));
  int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
  response += buffer;
  close(sock);
}

void testHttpRequests(const std::string& url, int t) {
  int totalRequests = 0;
  int totalBytes = 0;

  // Test with keep-alive connection
  std::cout << "Testing with keep-alive connection:" << std::endl;
  auto start = std::chrono::steady_clock::now();
  int sock;
  while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < t) {
    std::string response;
    sock = sendHttpRequestKeepAlive(url, response);
    totalRequests++;
    totalBytes += response.length();
  }
  close(sock);
  std::cout << "Total requests with keep-alive connection: " << totalRequests << std::endl;
  std::cout << "Total bytes with keep-alive connection: " << totalBytes << std::endl;

  // Test with close connection
  totalRequests = 0;
  totalBytes = 0;
  std::cout << "Testing with close connection:" << std::endl;
  start = std::chrono::steady_clock::now();
  while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < t) {
    std::string response;
    sendHttpRequest(url, response);
    totalRequests++;
    totalBytes += response.length();
  }
  std::cout << "Total requests with close connection: " << totalRequests << std::endl;
  std::cout << "Total bytes with close connection: " << totalBytes << std::endl;
}

int main() {
  std::string url = "127.0.0.1";
  int t = 10; // Time in seconds

  testHttpRequests(url, t);

  return 0;
}