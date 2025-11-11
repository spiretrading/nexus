#include <boost/optional/optional_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Stamp/StampMessage.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  auto make_message_buffer(
      const std::string& control, const std::string& business) {
    auto buffer = std::string();
    buffer += control;
    if(!business.empty()) {
      buffer += '\x1c';
      buffer += business;
    }
    return buffer;
  }
}

TEST_SUITE("StampMessage") {
  TEST_CASE("parse_control_only_no_business_content") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto buffer = make_message_buffer(control, "");
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(!message.get_business_content_data());
    REQUIRE(message.get_business_content_size() == 0);
  }

  TEST_CASE("parse_control_and_business_content") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto business = "\x1e""1=ABC""\x1e""2=123";
    auto buffer = make_message_buffer(control, business);
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(std::memcmp(message.get_business_content_data(), business,
      std::strlen(business)) == 0);
    REQUIRE(message.get_business_content_size() == std::strlen(business));
    REQUIRE(message.get_business_field<std::string>(1) == std::string("ABC"));
    REQUIRE(message.get_business_field<std::string>(2) == std::string("123"));
    REQUIRE(message.get_business_field<std::string>(3) == none);
  }

  TEST_CASE("get_business_field_with_order") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto business = "\x1e""1=ABC""\x1e""1.2=DEF";
    auto buffer = make_message_buffer(control, business);
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(message.get_business_field<std::string>(1) == std::string("ABC"));
    REQUIRE(
      message.get_business_field<std::string>(1, 2) == std::string("DEF"));
  }

  TEST_CASE("get_business_field_int") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto business = "\x1e""1=42""\x1e""2=100";
    auto buffer = make_message_buffer(control, business);
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(message.get_business_field<int>(1) == 42);
    REQUIRE(message.get_business_field<int>(2) == 100);
  }

  TEST_CASE("get_business_field_side") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto business = "\x1e""1=Buy""\x1e""2=Sell""\x1e""3=NA";
    auto buffer = make_message_buffer(control, business);
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(message.get_business_field<Side>(1) == Side(Side::BID));
    REQUIRE(message.get_business_field<Side>(2) == Side(Side::ASK));
    REQUIRE(message.get_business_field<Side>(3) == Side(Side::NONE));
  }

  TEST_CASE("get_business_field_ptime") {
    auto header = StampHeader();
    auto control = "CTRL";
    auto business = "\x1e""1=2024010112345601";
    auto buffer = make_message_buffer(control, business);
    auto message = StampMessage(header, buffer.data(), buffer.size());
    REQUIRE(message.get_business_field<ptime>(1) ==
      time_from_string("2024-01-01 12:34:56.010"));
  }
}
