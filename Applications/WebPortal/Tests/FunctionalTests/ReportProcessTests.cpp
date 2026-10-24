#include <array>
#include <cerrno>
#include <charconv>
#include <csignal>
#include <optional>
#include <string>
#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/process/process.hpp>
#include <boost/process/stdio.hpp>
#include <boost/scope/scope_exit.hpp>
#include <doctest/doctest.h>
#include <poll.h>
#include <unistd.h>

using namespace boost::asio;
using namespace boost::process;

namespace {
  std::optional<std::string> read_line(int descriptor) {
    constexpr auto TIMEOUT_MILLISECONDS = 10000;
    auto line = std::string();
    while(true) {
      auto event = pollfd(descriptor, POLLIN, 0);
      auto result = poll(&event, 1, TIMEOUT_MILLISECONDS);
      if(result == -1 && errno == EINTR) {
        continue;
      }
      if(result <= 0) {
        return std::nullopt;
      }
      auto value = char();
      auto size = read(descriptor, &value, sizeof(value));
      if(size == -1 && errno == EINTR) {
        continue;
      }
      if(size <= 0) {
        return std::nullopt;
      }
      if(value == '\n') {
        return line;
      }
      line.push_back(value);
    }
  }
}

TEST_SUITE("ReportProcess") {
  TEST_CASE("parent_lifetime") {
    auto mode = std::string("single");
    auto count = 1;
    auto is_crash = true;
    SUBCASE("abrupt_exit") {}
    SUBCASE("normal_exit") {
      is_crash = false;
    }
    SUBCASE("concurrent_reports") {
      mode = "pair";
      count = 2;
    }
    SUBCASE("closed_standard_descriptors") {
      mode = "closed";
    }
    auto input = std::array<int, 2>();
    REQUIRE(pipe(input.data()) == 0);
    auto close_input = boost::scope::scope_exit([&] {
      for(auto descriptor : input) {
        ::close(descriptor);
      }
    });
    auto output = std::array<int, 2>();
    REQUIRE(pipe(output.data()) == 0);
    auto close_output = boost::scope::scope_exit([&] {
      for(auto descriptor : output) {
        ::close(descriptor);
      }
    });
    auto context = io_context();
    auto parent = process(context, REPORT_PROCESS_FIXTURE,
      std::vector({mode}), process_stdio(input[0], output[1], stderr));
    ::close(input[0]);
    input[0] = -1;
    ::close(output[1]);
    output[1] = -1;
    auto groups = std::vector<pid_t>();
    auto cleanup = boost::scope::scope_exit([&] {
      auto error = boost::system::error_code();
      if(parent.running(error)) {
        parent.terminate(error);
      }
      parent.wait(error);
      for(auto id : groups) {
        kill(-id, SIGKILL);
      }
    });
    for(auto i = 0; i != count; ++i) {
      auto line = read_line(output[0]);
      REQUIRE(line);
      auto id = pid_t();
      auto parsed = std::from_chars(
        line->data(), line->data() + line->size(), id);
      REQUIRE(parsed.ec == std::errc());
      REQUIRE(parsed.ptr == line->data() + line->size());
      REQUIRE(id > 0);
      groups.push_back(id);
    }
    if(is_crash) {
      parent.terminate();
    } else {
      auto command = char(1);
      REQUIRE(write(input[1], &command, sizeof(command)) ==
        static_cast<ssize_t>(sizeof(command)));
    }
    constexpr auto TIMEOUT_MILLISECONDS = 10000;
    auto event = pollfd(output[0], POLLIN, 0);
    auto result = int();
    do {
      result = poll(&event, 1, TIMEOUT_MILLISECONDS);
    } while(result == -1 && errno == EINTR);
    REQUIRE(result > 0);
    auto value = char();
    REQUIRE(read(output[0], &value, sizeof(value)) == 0);
    groups.clear();
    auto exit_code = parent.wait();
    if(!is_crash) {
      REQUIRE(exit_code == 0);
    }
  }
}
