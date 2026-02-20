#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TickerInfo.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("TickerInfo") {
  TEST_CASE("stream") {
    auto info = TickerInfo();
    info.m_ticker = parse_ticker("TST.TSX");
    info.m_name = "Test Equity";
    info.m_instrument =
      Instrument(Asset(Asset::EQY, 111), CAD, Instrument::Type::SPOT);
    info.m_tick_size = Money::CENT;
    info.m_lot_size = 1;
    info.m_board_lot = 100;
    info.m_price_resolution = parse_money("0.01");
    info.m_quantity_resolution = 1;
    info.m_multiplier = 1;
    auto stream = std::stringstream();
    stream << info;
    auto expected = std::string("(") + to_string(info.m_ticker) + " " +
      info.m_name + " " +
      to_string(info.m_instrument) + " " +
      to_string(info.m_tick_size) + " " +
      to_string(info.m_lot_size) + " " +
      to_string(info.m_board_lot) + " " +
      to_string(info.m_price_resolution) + " " +
      to_string(info.m_quantity_resolution) + " " +
      to_string(info.m_multiplier) + ")";
    REQUIRE(stream.str() == expected);
    test_round_trip_shuttle(info);
  }
}
