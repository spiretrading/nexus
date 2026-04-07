#include <array>
#include <cstring>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Message.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Nexus;

TEST_SUITE("MoldUdp64Message") {
  TEST_CASE("parse_valid_message") {
    auto message_type = std::uint8_t(0xAB);
    auto data = "DATA";
    const auto data_length = sizeof("DATA") - 1;
    const auto length = std::uint16_t(1 + data_length);
    auto buffer = std::array<char, sizeof(std::uint16_t) + length>();
    auto length_be = native_to_big(length);
    std::memcpy(buffer.data(), &length_be, sizeof(std::uint16_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t), &message_type,
      sizeof(std::uint8_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t) + sizeof(std::uint8_t),
      data, data_length);
    auto message =
      MoldUdp64Message::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(message.m_length == length);
    REQUIRE(message.m_message_type == message_type);
    REQUIRE(std::memcmp(message.m_data, data, data_length) == 0);
  }

  TEST_CASE("parse_too_short_header") {
    const auto short_length = std::size_t(2);
    auto buffer = std::array<char, short_length>();
    REQUIRE_THROWS_AS(
      MoldUdp64Message::parse(std::string_view(buffer.data(), buffer.size())),
      MoldUdp64ParserException);
  }

  TEST_CASE("parse_too_short_for_length") {
    auto message_type = std::uint8_t(0xAB);
    auto data = "DATA";
    const auto data_length = sizeof("DATA") - 1;
    const auto length = std::uint16_t(1 + data_length + 5);
    auto buffer = std::array<char, sizeof(std::uint16_t) + 1 + data_length>();
    auto length_be = native_to_big(length);
    std::memcpy(buffer.data(), &length_be, sizeof(std::uint16_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t), &message_type,
      sizeof(std::uint8_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t) + sizeof(std::uint8_t),
      data, data_length);
    REQUIRE_THROWS_AS(
      MoldUdp64Message::parse(std::string_view(buffer.data(), buffer.size())),
      MoldUdp64ParserException);
  }

  TEST_CASE("parse_correct_endianness") {
    auto message_type = std::uint8_t(0xCD);
    auto data = "ENDIAN";
    const auto data_length = sizeof("ENDIAN") - 1;
    const auto length = std::uint16_t(1 + data_length);
    auto buffer = std::array<char, sizeof(std::uint16_t) + length>();
    auto length_be = native_to_big(length);
    std::memcpy(buffer.data(), &length_be, sizeof(std::uint16_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t), &message_type,
      sizeof(std::uint8_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t) + sizeof(std::uint8_t),
      data, data_length);
    auto message =
      MoldUdp64Message::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(message.m_length == length);
    REQUIRE(message.m_message_type == message_type);
  }

  TEST_CASE("parse_data_pointer") {
    auto message_type = std::uint8_t(0xEF);
    auto data = "PTR";
    const auto data_length = sizeof("PTR") - 1;
    const auto length = std::uint16_t(1 + data_length);
    auto buffer = std::array<char, sizeof(std::uint16_t) + length>();
    auto length_be = native_to_big(length);
    std::memcpy(buffer.data(), &length_be, sizeof(std::uint16_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t), &message_type,
      sizeof(std::uint8_t));
    std::memcpy(buffer.data() + sizeof(std::uint16_t) + sizeof(std::uint8_t),
      data, data_length);
    auto message =
      MoldUdp64Message::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(message.m_data == buffer.data() + sizeof(std::uint16_t) +
      sizeof(std::uint8_t));
    REQUIRE(std::memcmp(message.m_data, data, data_length) == 0);
  }
}
