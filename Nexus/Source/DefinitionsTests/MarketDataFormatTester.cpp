#include <array>
#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/Definitions/TickerStatus.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("MarketDataFormat") {
  TEST_CASE("quotes") {
    auto timestamp = time_from_string("2026-10-16 09:30:00");
    auto bid = make_bid(10 * Money::ONE, 100);
    auto ask = make_ask(11 * Money::ONE, 200);
    REQUIRE(std::format("{}", bid) == "(10.00 100 BID)");
    REQUIRE(std::format("{:>17}", bid) == "  (10.00 100 BID)");
    REQUIRE(to_string(bid) == "(10.00 100 BID)");
    auto bbo = BboQuote(bid, ask, timestamp);
    REQUIRE(std::format("{}", bbo) ==
      "((10.00 100 BID) (11.00 200 ASK) 2026-Oct-16 09:30:00)");
    REQUIRE(to_string(bbo) ==
      "((10.00 100 BID) (11.00 200 ASK) 2026-Oct-16 09:30:00)");
    auto book = BookQuote("MPID", true, Venues::TSX, ask, timestamp);
    REQUIRE(std::format("{}", book) ==
      "(MPID true TSX (11.00 200 ASK) 2026-Oct-16 09:30:00)");
    book.m_is_primary_mpid = false;
    REQUIRE(std::format("{}", book) ==
      "(MPID false TSX (11.00 200 ASK) 2026-Oct-16 09:30:00)");
  }

  TEST_CASE("trade_conditions") {
    using Type = TimeAndSale::Condition::Type;
    auto cases = std::array{
      std::pair(Type::REGULAR, "@"), std::pair(Type::OPEN, "OPEN"),
      std::pair(Type::CLOSE, "CLOSE"), std::pair(Type::REOPEN, "REOPEN"),
      std::pair(Type::AUCTION, "AUCTION"),
      std::pair(Type::CORRECTION, "CORRECTION"),
      std::pair(Type::CANCELLATION, "CANCELLATION"),
      std::pair(Type::NONE, "NONE")};
    for(auto& [type, name] : cases) {
      REQUIRE(std::format("{}", type) == name);
      REQUIRE(std::format("{}", Type(type)) == name);
      REQUIRE(to_string(type) == name);
      REQUIRE(to_string(Type(type)) == name);
    }
    REQUIRE(std::format("{}", static_cast<Type::Type>(99)) == "NONE");
    auto condition = TimeAndSale::Condition(Type::REGULAR, "T");
    REQUIRE(std::format("{}", condition) == "(@ T)");
    REQUIRE(std::format("{:>7}", condition) == "  (@ T)");
    REQUIRE(to_string(condition) == "(@ T)");
    auto sale = TimeAndSale(time_from_string("2026-10-16 09:30:00"),
      10 * Money::ONE, 100, condition, "TSX", "BUY", "SELL");
    REQUIRE(std::format("{}", sale) ==
      "(2026-Oct-16 09:30:00 10.00 100 (@ T) TSX BUY SELL)");
    REQUIRE(to_string(sale) ==
      "(2026-Oct-16 09:30:00 10.00 100 (@ T) TSX BUY SELL)");
  }

  TEST_CASE("status_flags") {
    using Flag = TickerStatus::Flag;
    REQUIRE(std::format("{}", Flag::NONE) == "NONE");
    REQUIRE(std::format("{}", Flag::IS_AUCTION) == "IS_AUCTION");
    REQUIRE(std::format("{}", Flag::IS_CONTINUOUS) ==
      "(IS_MATCHING | IS_ACCEPTING_ORDERS | IS_ACCEPTING_CANCELS)");
    REQUIRE(std::format("{}", Flag::IS_CONTINUOUS | Flag::IS_AUCTION) ==
      "(IS_MATCHING | IS_ACCEPTING_ORDERS | "
      "IS_ACCEPTING_CANCELS | IS_AUCTION)");
    REQUIRE(std::format("{}", static_cast<Flag>(0x80)) == "NONE");
    REQUIRE(std::format("{:>6}", Flag::NONE) == "  NONE");
  }

  TEST_CASE("records") {
    auto timestamp = time_from_string("2026-10-16 09:30:00");
    auto ticker = parse_ticker("ABC.TSX");
    auto info = TickerInfo(ticker, "Company", "Sector", 100);
    REQUIRE(std::format("{}", info) == "(ABC.TSX Company Sector 100)");
    REQUIRE(to_string(info) == "(ABC.TSX Company Sector 100)");
    auto imbalance = OrderImbalance(ticker, Side::BID, 100,
      10 * Money::ONE, timestamp);
    REQUIRE(std::format("{}", imbalance) ==
      "(ABC.TSX BID 100 10.00 2026-Oct-16 09:30:00)");
    REQUIRE(to_string(imbalance) ==
      "(ABC.TSX BID 100 10.00 2026-Oct-16 09:30:00)");
    auto status = TickerStatus(
      Venues::TSX, "OPEN", TickerStatus::Flag::IS_AUCTION, timestamp);
    REQUIRE(std::format("{}", status) ==
      "(TSX OPEN IS_AUCTION 2026-Oct-16 09:30:00)");
    REQUIRE(to_string(status) ==
      "(TSX OPEN IS_AUCTION 2026-Oct-16 09:30:00)");
    auto event = TradingSchedule::Event("OPEN", timestamp);
    REQUIRE(std::format("{}", event) == "(OPEN 2026-Oct-16 09:30:00)");
    REQUIRE(to_string(event) == "(OPEN 2026-Oct-16 09:30:00)");
    REQUIRE(std::format("{:.5}", event) == "(OPEN");
  }

  TEST_CASE("custom_venue") {
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = Venues::TSX;
    venue.m_display_name = "CUSTOM";
    venues.add(venue);
    auto locale = make_locale(venues);
    auto timestamp = time_from_string("2026-10-16 09:30:00");
    auto ticker = parse_ticker("ABC.TSX");
    auto info = TickerInfo(ticker, "Company", "Sector", 100);
    REQUIRE(std::format(locale, "{}", info) ==
      "(ABC.CUSTOM Company Sector 100)");
    auto output = std::ostringstream();
    output << venues << info;
    REQUIRE(output.str() == "(ABC.CUSTOM Company Sector 100)");
    auto book = BookQuote(
      "MPID", true, Venues::TSX, make_bid(10 * Money::ONE, 100), timestamp);
    REQUIRE(std::format(locale, "{}", book) ==
      "(MPID true CUSTOM (10.00 100 BID) 2026-Oct-16 09:30:00)");
    auto book_output = std::ostringstream();
    book_output << venues << book;
    REQUIRE(book_output.str() ==
      "(MPID true CUSTOM (10.00 100 BID) 2026-Oct-16 09:30:00)");
    auto imbalance = OrderImbalance(ticker, Side::BID, 100,
      10 * Money::ONE, timestamp);
    REQUIRE(std::format(locale, "{}", imbalance) ==
      "(ABC.CUSTOM BID 100 10.00 2026-Oct-16 09:30:00)");
    auto imbalance_output = std::ostringstream();
    imbalance_output << venues << imbalance;
    REQUIRE(imbalance_output.str() ==
      "(ABC.CUSTOM BID 100 10.00 2026-Oct-16 09:30:00)");
    auto status = TickerStatus(
      Venues::TSX, "OPEN", TickerStatus::Flag::IS_AUCTION, timestamp);
    REQUIRE(std::format(locale, "{}", status) ==
      "(CUSTOM OPEN IS_AUCTION 2026-Oct-16 09:30:00)");
    auto status_output = std::ostringstream();
    status_output << venues << status;
    REQUIRE(status_output.str() ==
      "(CUSTOM OPEN IS_AUCTION 2026-Oct-16 09:30:00)");
  }
}
