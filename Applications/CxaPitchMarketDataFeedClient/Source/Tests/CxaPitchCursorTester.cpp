#include <array>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"

using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("CxaPitchCursor") {
  TEST_CASE("read_integers") {
    auto data = std::array<char, 15>{
      0x2A,
      0x2C, 0x01,
      static_cast<char>(0xBC), 0x02, 0x00, 0x00,
      0x15, static_cast<char>(0xCD), 0x5B, 0x07, 0x00, 0x00, 0x00, 0x00};
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_uint8() == 42);
    REQUIRE(cursor.read_uint16() == 300);
    REQUIRE(cursor.read_uint32() == 700);
    REQUIRE(cursor.read_uint64() == 123456789);
  }

  TEST_CASE("read_timestamp") {
    auto data = std::array<char, 8>{static_cast<char>(0xF0), 0x77,
      static_cast<char>(0xBB), static_cast<char>(0xCE), 0x2A, 0x6A, 0x62, 0x16};
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_timestamp() ==
      time_from_string("2021-02-10 14:45:48.641622"));
  }

  TEST_CASE("read_price") {
    auto data = std::array<char, 24>{
      0x15, static_cast<char>(0xCD), 0x5B, 0x07, 0x00, 0x00, 0x00, 0x00,
      static_cast<char>(0x80), static_cast<char>(0x96), static_cast<char>(0x98),
      0x00, 0x00, 0x00, 0x00, 0x00,
      static_cast<char>(0xA0), static_cast<char>(0x86), 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00};
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_price() == parse_money("12.3456789"));
    REQUIRE(cursor.read_price() == Money::ONE);
    REQUIRE(cursor.read_price() == Money::CENT);
  }

  TEST_CASE("read_side") {
    auto data = std::array<char, 3>{'B', 'S', 'X'};
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_side() == Side::BID);
    REQUIRE(cursor.read_side() == Side::ASK);
    REQUIRE(cursor.read_side() == Side::NONE);
  }

  TEST_CASE("read_price_decimal_scale") {
    auto data = std::array<char, 8>();
    auto price = std::uint64_t(20100000);
    for(auto& byte : data) {
      byte = static_cast<char>(price & 0xFF);
      price >>= 8;
    }
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_price() == 201 * Money::CENT);
    data[0] += 1;
    cursor = CxaPitchCursor(data.data());
    REQUIRE(
      cursor.read_price() == Money(Quantity::from_representation(2010000.1)));
  }

  TEST_CASE("read_text") {
    auto data = std::array<char, 16>{
      'Z', 'V', 'Z', 'T', ' ', ' ',
      '1', '2', '3', '4',
      ' ', ' ', ' ', ' ',
      'A', 'B'};
    auto cursor = CxaPitchCursor(data.data());
    REQUIRE(cursor.read_text(6) == "ZVZT");
    REQUIRE(cursor.read_text(4) == "1234");
    REQUIRE(cursor.read_text(4) == "");
    REQUIRE(cursor.read_text(2) == "AB");
  }

  TEST_CASE("skip") {
    auto data = std::array<char, 5>{0x01, 0x02, 0x03, 0x04, 0x05};
    auto cursor = CxaPitchCursor(data.data());
    cursor.skip(4);
    REQUIRE(cursor.read_uint8() == 5);
  }
}
