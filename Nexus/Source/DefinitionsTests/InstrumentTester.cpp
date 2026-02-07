#include <array>
#include <functional>
#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Instrument.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("Instrument") {
  TEST_CASE("type_stream") {
    auto cases = std::array<std::pair<Instrument::Type, const char*>, 7>({
      {Instrument::Type::NONE, "NONE"},
      {Instrument::Type::SPOT, "SPT"},
      {Instrument::Type::FORWARD, "FWD"},
      {Instrument::Type::FUTURE, "FUT"},
      {Instrument::Type::PERPETUAL, "PERP"},
      {Instrument::Type::OPTION, "OPT"},
      {Instrument::Type::SWAP, "SWP"}
    });
    for(auto& entry : cases) {
      auto stream = std::stringstream();
      stream << entry.first;
      REQUIRE(stream.str() == entry.second);
      REQUIRE(!stream.fail());
    }
    auto stream = std::stringstream();
    stream << static_cast<Instrument::Type>(255);
    REQUIRE(stream.fail());
  }

  TEST_CASE("hash") {
    auto instrument = Instrument();
    instrument.m_base = Asset(Asset::CCY, 1);
    instrument.m_quote = Asset(Asset::CCY, 2);
    instrument.m_type = Instrument::Type::SPOT;
    auto hash = std::hash<Instrument>()(instrument);
    auto seed = std::hash<Asset>()(instrument.m_base);
    auto quote_hash = std::hash<Asset>()(instrument.m_quote);
    seed ^= quote_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    auto type_hash =
      std::hash<int>()(static_cast<int>(instrument.m_type));
    seed ^= type_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    REQUIRE(hash == seed);
  }

  TEST_CASE("shuttle") {
    auto instrument = Instrument();
    instrument.m_base = Asset(Asset::EQY, 123);
    instrument.m_quote = Asset(Asset::CCY, 456);
    instrument.m_type = Instrument::Type::PERPETUAL;
    test_round_trip_shuttle(instrument);
  }
}
