#include <sstream>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshot.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

TEST_SUITE("OtcLinkSnapshot") {
  TEST_CASE("stream") {
    auto out = std::ostringstream();
    SUBCASE("spin_start") {
      auto message = OtcLinkSpinStart();
      message.m_sequence = 1;
      message.m_type = OtcLinkSpinType::MARKET_DATA;
      message.m_timestamp = 3;
      message.m_last_sequence = 4;
      out << message;
      REQUIRE(out.str() == "(spin_start 1 2 3 4)");
    }
    SUBCASE("spin_end") {
      auto message = OtcLinkSpinEnd();
      message.m_sequence = 1;
      message.m_type = OtcLinkSpinType::MARKET_DATA;
      message.m_count = 3;
      message.m_timestamp = 4;
      message.m_last_sequence = 5;
      out << message;
      REQUIRE(out.str() == "(spin_end 1 2 3 4 5)");
    }
  }

  TEST_CASE("spin_messages") {
    auto make = [] (bool is_end) {
      auto payload = SharedBuffer();
      append(payload, endian::native_to_big(std::uint32_t(42)));
      append(payload, static_cast<std::uint8_t>(OtcLinkSpinType::MARKET_DATA));
      if(is_end) {
        append(payload, endian::native_to_big(std::uint32_t(30)));
      }
      append(payload, endian::native_to_big(std::uint64_t(1790000000123)));
      append(payload, endian::native_to_big(std::uint32_t(900)));
      return payload;
    };
    auto payload = make(false);
    auto message = OtcLinkMessage(0, OtcLinkSpinStart::TYPE,
      std::string_view(payload.get_data(), payload.get_size()));
    auto start = OtcLinkSpinStart::parse(message);
    REQUIRE(start.m_sequence == 42);
    REQUIRE(start.m_type == OtcLinkSpinType::MARKET_DATA);
    REQUIRE(start.m_timestamp == 1790000000123);
    REQUIRE(start.m_last_sequence == 900);
    for(auto size = std::size_t(0); size < payload.get_size(); ++size) {
      message.m_payload = std::string_view(payload.get_data(), size);
      REQUIRE_THROWS_AS(OtcLinkSpinStart::parse(message),
        OtcLinkParserException);
    }
    payload = make(true);
    message.m_type = OtcLinkSpinEnd::TYPE;
    message.m_payload =
      std::string_view(payload.get_data(), payload.get_size());
    auto end = OtcLinkSpinEnd::parse(message);
    REQUIRE(end.m_sequence == 42);
    REQUIRE(end.m_type == OtcLinkSpinType::MARKET_DATA);
    REQUIRE(end.m_count == 30);
    REQUIRE(end.m_timestamp == 1790000000123);
    REQUIRE(end.m_last_sequence == 900);
    REQUIRE_THROWS_AS(
      OtcLinkSpinStart::parse(message), OtcLinkParserException);
    for(auto size = std::size_t(0); size < payload.get_size(); ++size) {
      message.m_payload = std::string_view(payload.get_data(), size);
      REQUIRE_THROWS_AS(OtcLinkSpinEnd::parse(message), OtcLinkParserException);
    }
    auto type = std::uint8_t(0);
    payload.write(sizeof(std::uint32_t), &type, sizeof(type));
    message.m_payload =
      std::string_view(payload.get_data(), payload.get_size());
    REQUIRE_THROWS_AS(OtcLinkSpinEnd::parse(message), OtcLinkParserException);
  }
}
