# Webserver Project

This project refactors the [Muduo](https://github.com/chenshuo/recipes) network library using C++11. It includes an implementation of an HTTP server on top of the refactored library.

## Features
- Event-driven I/O multiplexing framework
- Support multithreading
- Implement a fully functional HTTP server
- Optimized with window-LFU caching algorithm

## Bulid

1. Clone the repository:
  ```sh
  git clone https://github.com/Jianglandelong/webserver.git
  ```
2. Build the project:
  ```sh
  cd webserver
  cmake .
  make
  ```

## Usage

**Run the server:**
```sh
./bin/HttpServer [numThreads] 
```

Default port is 9981

**Test the server:**

[Web Bench](http://home.tiscali.cz/~cz210552/webbench.html) is a simple benchmarking tool that can be used to test the server.
```sh
webbench -c [numClients] -t [time] http://[ip]:[port]/
```