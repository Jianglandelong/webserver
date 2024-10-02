#include <iostream>
#include <chrono>
#include <thread>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
  size_t totalSize = size * nmemb;
  response->append(static_cast<char*>(contents), totalSize);
  return totalSize;
}

void sendGetRequest(const std::string& url, bool keepAlive, int duration, int& requestCount, int& byteCount) {
  CURL* curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    if (keepAlive) {
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    }
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count() < duration) {
      CURLcode res = curl_easy_perform(curl);
      if (res == CURLE_OK) {
        requestCount++;
        byteCount += response.size();
      }
      response.clear();
    }
    curl_easy_cleanup(curl);
  }
}

int main(int argc, char* argv[]) {
  std::string url = "http://127.0.0.1:9981/"; // Replace with your desired URL
  bool keepAlive = true; // Set to false if you don't want to use keep-alive
  int duration = 10; // Set the duration in seconds

  // Check if keepAlive and duration are specified as command line arguments
  if (argc >= 3) {
    keepAlive = std::stoi(argv[1]);
    duration = std::stoi(argv[2]);
  }

  int requestCount = 0;
  int byteCount = 0;
  sendGetRequest(url, keepAlive, duration, requestCount, byteCount);
  std::cout << "Total requests: " << requestCount << std::endl;
  std::cout << "Total bytes: " << byteCount << std::endl;
  return 0;
}