#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/process/stdio.hpp>
#include "../Source/ReportProcessGroup.hpp"

using namespace boost::asio;
using namespace boost::process;

int main(int argc, char** argv) {
  if(argc != 2) {
    return 1;
  }
  if(std::strcmp(argv[1], "report") == 0) {
    auto descendant = fork();
    if(descendant == -1) {
      return 1;
    }
    if(descendant != 0) {
      std::cout << getpgrp() << std::endl;
    }
    while(true) {
      pause();
    }
  }
  try {
    auto context = io_context();
    auto groups = std::vector<std::unique_ptr<ProcessGroup>>();
    auto processes = std::vector<process>();
    auto count = 1;
    if(std::strcmp(argv[1], "pair") == 0) {
      count = 2;
    }
    for(auto i = 0; i != count; ++i) {
      if(std::strcmp(argv[1], "closed") == 0) {
        auto descriptors = std::array<int, STDERR_FILENO + 1>();
        for(auto j = STDIN_FILENO; j <= STDERR_FILENO; ++j) {
          descriptors[j] = dup(j);
          if(descriptors[j] == -1) {
            return 1;
          }
        }
        for(auto j = STDIN_FILENO; j <= STDERR_FILENO; ++j) {
          ::close(j);
        }
        groups.push_back(std::make_unique<ProcessGroup>());
        for(auto j = STDIN_FILENO; j <= STDERR_FILENO; ++j) {
          if(dup2(descriptors[j], j) == -1) {
            return 1;
          }
          ::close(descriptors[j]);
        }
      } else {
        groups.push_back(std::make_unique<ProcessGroup>());
      }
      auto launcher = default_process_launcher();
      processes.push_back(launcher(context.get_executor(), argv[0],
        std::vector<std::string>({"report"}),
        process_stdio(nullptr, stdout, stderr), *groups.back()));
    }
    auto command = char();
    while(read(STDIN_FILENO, &command, sizeof(command)) == -1 &&
        errno == EINTR) {}
  } catch(const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
    return 1;
  }
  return 0;
}
