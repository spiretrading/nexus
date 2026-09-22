#include <ostream>
#include <string>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"

using namespace Nexus;

TEST_SUITE("MoldUdp64Message") {
  TEST_CASE("parse") {
    SUBCASE("payload") {
      auto source = std::string_view("\x00\x04\x00\xFFXY", 6);
      auto message = MoldUdp64Message::parse(source);
      REQUIRE(message.m_length == 4);
      REQUIRE(message.m_data == source.data() + sizeof(std::uint16_t));
      REQUIRE(std::string_view(message.m_data, message.m_length) ==
        source.substr(sizeof(std::uint16_t)));
    }
    SUBCASE("empty") {
      auto source = std::string_view("\x00\x00", 2);
      auto message = MoldUdp64Message::parse(source);
      REQUIRE(message.m_length == 0);
      REQUIRE(message.m_data == source.data() + source.size());
    }
    SUBCASE("truncated") {
      auto source = std::string_view("\x00\x04\x00\xFFXY", 6);
      for(auto size = std::size_t(0); size != source.size(); ++size) {
        REQUIRE_THROWS_AS(MoldUdp64Message::parse(source.substr(0, size)),
          MoldUdp64ParserException);
      }
    }
    SUBCASE("following_message") {
      auto source = std::string_view("\x00\x01X\x00\x01Y", 6);
      auto message = MoldUdp64Message::parse(source);
      REQUIRE(message.m_length == 1);
      REQUIRE(std::string_view(message.m_data, message.m_length) == "X");
    }
    SUBCASE("maximum_length") {
      auto source = std::string("\xFF\xFF", 2);
      source.append(65535, 'X');
      auto message = MoldUdp64Message::parse(source);
      REQUIRE(message.m_length == 65535);
      REQUIRE(std::string_view(message.m_data, message.m_length) ==
        std::string_view(source).substr(sizeof(std::uint16_t)));
    }
  }
}
