#include <functional>
#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Currency.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("Asset") {
  TEST_CASE("default_constructor") {
    auto asset = Asset();
    REQUIRE(asset.get_id() == 0);
    REQUIRE(asset.get_type() == 0);
    REQUIRE_FALSE(static_cast<bool>(asset));
  }

  TEST_CASE("value_constructor") {
    auto value = std::uint64_t(0x123456789ABCDEF0ULL);
    auto asset = Asset(value);
    REQUIRE(asset.get_id() == value);
    REQUIRE(asset.get_type() == 0x1234);
    REQUIRE(static_cast<bool>(asset));
  }

  TEST_CASE("type_and_code_constructor") {
    auto code = std::uint64_t(0x000056789ABCDEF0ULL);
    auto asset = Asset(Asset::CURRENCY, code);
    auto expected = (static_cast<std::uint64_t>(Asset::CURRENCY) << 48) |
      (code & 0x0000FFFFFFFFFFFFULL);
    REQUIRE(asset.get_type() == Asset::CURRENCY);
    REQUIRE(asset.get_code() == code);
    REQUIRE(asset.get_id() == expected);
  }

  TEST_CASE("string_type_constructor") {
    auto code = std::uint64_t(42);
    auto asset = Asset("EQY", code);
    auto expected = (static_cast<std::uint64_t>(Asset::EQUITY) << 48) |
      (code & 0x0000FFFFFFFFFFFFULL);
    REQUIRE(asset.get_type() == Asset::EQUITY);
    REQUIRE(asset.get_code() == code);
    REQUIRE(asset.get_id() == expected);
  }

  TEST_CASE("encode_decode_asset_type") {
    auto code = encode_asset_type("CCY");
    auto decoded = decode_asset_type(code);
    REQUIRE(code == Asset::CURRENCY);
    REQUIRE(decoded == "CCY");
  }

  TEST_CASE("hash") {
    auto value = std::uint64_t(0x123456789ABCDEF0ULL);
    auto asset = Asset(value);
    auto hash = std::hash<Asset>()(asset);
    auto expected = std::hash<std::uint64_t>()(value);
    REQUIRE(hash == expected);
  }

  TEST_CASE("stream") {
    auto asset = Asset(Asset::EQY, 42);
    auto stream = std::stringstream();
    stream << asset;
    REQUIRE(stream.str() == "EQY(42)");
    auto currency = Asset(Asset::CCY, 124);
    stream.str("");
    stream << currency;
    REQUIRE(stream.str() == "CAD");
  }

  TEST_CASE("shuttle") {
    auto value = Asset(Asset::CCY, std::uint64_t(42));
    test_round_trip_shuttle(value);
  }
}
