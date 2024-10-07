#pragma once

#include <algorithm>
#include <fcntl.h>
#include <list>
#include <cmath>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

namespace webserver {

struct Node
{
  std::string key;
  char* value;
  int freq;
  Node(std::string file_name, char* file_addr, int _freq) : key(file_name), value(file_addr), freq(_freq) {}
};

class LFUCache
{
public:
  int capacity;
  int histroy_window;
  std::unordered_map<int, std::list<Node>> freq_table;
  std::unordered_map<std::string, std::list<Node>::iterator> key_table;
  std::list<std::string> access_history;
  int min_freq;

  LFUCache(int _capacity, int window)
  {
    capacity = _capacity;
    histroy_window = window;
    min_freq = 0;
  }

  char* get(const std::string &file_name)
  {
    auto key_iter = key_table.find(file_name);
    if (key_iter == key_table.end())
    {
      return put(file_name);
    }
    auto iter = key_iter->second;
    char* val = iter->value;
    int freq = iter->freq;
    freq_table[freq].erase(iter);
    if (freq_table[freq].empty() && min_freq == freq)
    {
      min_freq = freq + 1;
    }
    freq++;
    freq_table[freq].emplace_front(file_name, val, freq);
    key_table[file_name] = freq_table[freq].begin();
    update_window(file_name);
    return val;
  }

  char* put(const std::string &file_name)
  {
    if (key_table.size() == capacity)
    {
      auto rm_key = freq_table[min_freq].back().key;
      auto rm_file_addr = key_table[rm_key]->value;
      freq_table[min_freq].pop_back();
      key_table.erase(rm_key);
      unmap(rm_key);
    }
    min_freq = 1;
    auto file_addr = get_file(file_name);
    freq_table[min_freq].emplace_front(file_name, file_addr, min_freq);
    key_table[file_name] = freq_table[min_freq].begin();
    return file_addr;
  }

  char* get_file(const std::string &file_name) {
    auto fd = ::open(file_name.c_str(), O_RDONLY);
    struct stat file_stat;
    stat(file_name.c_str(), &file_stat);
    char* file_addr = (char*)mmap(0, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);
    return file_addr;
  }

  void update_window(const std::string &file_name) {
    if (access_history.size() == histroy_window) {
      auto obsolete_access = access_history.front();
      access_history.pop_front();
      auto key_iter = key_table.find(obsolete_access);
      if (key_iter != key_table.end()) {
        auto freq = key_iter->second->freq;
        freq_table[freq].erase(key_iter->second);
        if (freq_table[freq].empty() && freq == min_freq) {
          min_freq--;
          if (min_freq == 0) {
            find_min_freq();
          }
        }
        freq--;
        if (freq > 0) {
          char *val = key_iter->second->value;
          freq_table[freq].emplace_front(file_name, val, freq);
          key_table[file_name] = freq_table[freq].begin();
        } else {
          key_table.erase(key_iter);
          unmap(key_iter->first);
        }
      }
    }
    access_history.push_back(file_name);
  }

  void find_min_freq() {
    min_freq = INT32_MAX;
    for (const auto &iter : freq_table) {
      min_freq = std::min(min_freq, iter.first);
    }
  }

  void unmap(const std::string &file_name) {
    auto file_addr = key_table[file_name]->value;
    struct stat file_stat;
    stat(file_name.c_str(), &file_stat);
    munmap(file_addr, file_stat.st_size);
  }
};

}