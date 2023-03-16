#include <doctest/doctest.h>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/KeyBindings/OrderTaskMatch.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("OrderTaskMatch") {
  TEST_CASE("match_name") {
    auto value = QString("hello");
    REQUIRE(matches(value, "h"));
    REQUIRE(matches(value, "he"));
    REQUIRE(matches(value, "Hel"));
    REQUIRE(matches(value, "hello"));
    REQUIRE(matches(value, "e"));
    REQUIRE(matches(value, "El"));
    REQUIRE(matches(value, "llo"));
    REQUIRE(!matches(value, "ho"));
    REQUIRE(!matches(value, "hello w"));
    REQUIRE(!matches(value, "w"));
  }

  TEST_CASE("match_country") {
    auto country = CountryCode(840);
    SUBCASE("match_country_letter_code") {
      REQUIRE(matches(country, "u"));
      REQUIRE(matches(country, "Us"));
      REQUIRE(matches(country, "sa"));
      REQUIRE(matches(country, "USA"));
    }
    SUBCASE("match_country_name") {
      REQUIRE(matches(country, "un"));
      REQUIRE(matches(country, "Uni"));
      REQUIRE(matches(country, "united"));
      REQUIRE(matches(country, "united s"));
      REQUIRE(matches(country, "United States"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(country, "840"));
      REQUIRE(!matches(country, "C"));
      REQUIRE(!matches(country, "Canada"));
      REQUIRE(!matches(country, "UA"));
      REQUIRE(!matches(country, "UnitedStates"));
      REQUIRE(!matches(country, "States"));
      REQUIRE(!matches(CountryCode::NONE, "USA"));
    }
  }

  TEST_CASE("match_market") {
    auto market = MarketCode("XCNQ");
    SUBCASE("match_market_code") {
      REQUIRE(matches(market, "x"));
      REQUIRE(matches(market, "xc"));
      REQUIRE(matches(market, "xcNQ"));
    }
    SUBCASE("match_market_display_name") {
      REQUIRE(matches(market, "C"));
      REQUIRE(matches(market, "cs"));
      REQUIRE(matches(market, "SE"));
      REQUIRE(matches(market, "CSE"));
    }
    SUBCASE("match_market_description") {
      REQUIRE(matches(market, "Canadian"));
      REQUIRE(matches(market, "Canadian Securities"));
      REQUIRE(matches(market, "Canadian Securities Exchange"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(market, "xc nq"));
      REQUIRE(!matches(market, "xnq"));
      REQUIRE(!matches(market, "Canada"));
      REQUIRE(!matches(market, "ce"));
      REQUIRE(!matches(market, "csey"));
      REQUIRE(!matches(MarketCode(), "XCNQ"));
    }
  }

  TEST_CASE("match_security") {
    auto security = *ParseWildCardSecurity("MG.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    SUBCASE("match_security_display_name") {
      REQUIRE(matches(security, "m"));
      REQUIRE(matches(security, "mg"));
      REQUIRE(matches(security, "MG."));
      REQUIRE(matches(security, "MG.TS"));
      REQUIRE(matches(security, "MG.TSX"));
      REQUIRE(matches(security, "TS"));
      REQUIRE(matches(security, "tsx"));
    }
    SUBCASE("match_security_market") {
      REQUIRE(matches(security, "x"));
      REQUIRE(matches(security, "XT"));
      REQUIRE(matches(security, "XTse"));
      REQUIRE(matches(security, "t"));
      REQUIRE(matches(security, "TSX"));
      REQUIRE(matches(security, "Toronto"));
      REQUIRE(matches(security, "Toronto Stock"));
      REQUIRE(matches(security, "Toronto Stock Exchange"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(security, "MG "));
      REQUIRE(!matches(security, "MGTSX"));
      REQUIRE(!matches(security, "MFC"));
      REQUIRE(!matches(security, "TSXV"));
      REQUIRE(!matches(security, "Stock"));
      REQUIRE(!matches(Security(), "MG.TSX"));
    }
  }

  TEST_CASE("match_region") {
    auto region = Region("Test");
    region = region + CountryCode(840);
    region = region + GetDefaultMarketDatabase().FromCode("XCNQ");
    region = region + *ParseWildCardSecurity("MG.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    SUBCASE("match_region_name") {
      REQUIRE(matches(region, "t"));
      REQUIRE(matches(region, "te"));
      REQUIRE(matches(region, "test"));
    }
    SUBCASE("match_region_country") {
      REQUIRE(matches(region, "u"));
      REQUIRE(matches(region, "Us"));
      REQUIRE(matches(region, "USA"));
      REQUIRE(matches(region, "united"));
      REQUIRE(matches(region, "United States"));
    }
    SUBCASE("match_region_market") {
      REQUIRE(matches(region, "x"));
      REQUIRE(matches(region, "xc"));
      REQUIRE(matches(region, "XCNQ"));
      REQUIRE(matches(region, "CS"));
      REQUIRE(matches(region, "CSE"));
      REQUIRE(matches(region, "Canadian"));
      REQUIRE(matches(region, "Canadian Securities Exchange"));
    }
    SUBCASE("match_region_security") {
      REQUIRE(matches(region, "m"));
      REQUIRE(matches(region, "mg"));
      REQUIRE(matches(region, "MG.TSX"));
      REQUIRE(matches(region, "TS"));
      REQUIRE(matches(region, "XT"));
      REQUIRE(matches(region, "XTse"));
      REQUIRE(matches(region, "TSX"));
      REQUIRE(matches(region, "Toronto"));
      REQUIRE(matches(region, "Toronto Stock Exchange"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(region, "840"));
      REQUIRE(!matches(region, "B"));
      REQUIRE(!matches(region, "st"));
      REQUIRE(!matches(region, "test "));
      REQUIRE(!matches(region, "Canada"));
      REQUIRE(!matches(region, "TSXV"));
      REQUIRE(!matches(region, "UA"));
      REQUIRE(!matches(Region(), "USA"));
    }
  }

  TEST_CASE("match_destination") {
    auto destination = Destination("AMEX");
    SUBCASE("match_destination_id") {
      REQUIRE(matches(destination, "a"));
      REQUIRE(matches(destination, "AM"));
      REQUIRE(matches(destination, "mex"));
      REQUIRE(matches(destination, "x"));
      REQUIRE(matches(destination, "AMEX"));
    }
    SUBCASE("match_destination_description") {
      REQUIRE(matches(destination, "n"));
      REQUIRE(matches(destination, "NY"));
      REQUIRE(matches(destination, "nYse"));
      REQUIRE(matches(destination, "NYSE mkt"));
      REQUIRE(matches(destination, "NYSE MKT LLC"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(destination, "B"));
      REQUIRE(!matches(destination, "AS"));
      REQUIRE(!matches(destination, "amexa"));
      REQUIRE(!matches(destination, "NYS "));
      REQUIRE(!matches(destination, "NYSEMKT"));
      REQUIRE(!matches(destination, "LLC"));
      REQUIRE(!matches(Destination(), "AMEX"));
    }
  }

  TEST_CASE("match_order_type") {
    auto order_type = OrderType(OrderType::MARKET);
    REQUIRE(matches(order_type, "m"));
    REQUIRE(matches(order_type, "Ma"));
    REQUIRE(matches(order_type, "maR"));
    REQUIRE(matches(order_type, "ark"));
    REQUIRE(matches(order_type, "KET"));
    REQUIRE(matches(order_type, "T"));
    REQUIRE(matches(order_type, "MARKET"));
    REQUIRE(!matches(order_type, "MO"));
    REQUIRE(!matches(order_type, "mar ket"));
    REQUIRE(!matches(order_type, "STOP"));
    REQUIRE(!matches(OrderType(), "MARKET"));
  }

  TEST_CASE("match_side") {
    auto side = Side(Side::ASK);
    SUBCASE("match_side_display_name") {
      REQUIRE(matches(side, "s"));
      REQUIRE(matches(side, "sE"));
      REQUIRE(matches(side, "Sel"));
      REQUIRE(matches(side, "sell"));
      REQUIRE(matches(side, "ll"));
    }
    SUBCASE("match_side_value") {
      REQUIRE(matches(side, "a"));
      REQUIRE(matches(side, "As"));
      REQUIRE(matches(side, "Ask"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(side, "sa"));
      REQUIRE(!matches(side, "selll"));
      REQUIRE(!matches(side, "se ll"));
      REQUIRE(!matches(side, "askk"));
      REQUIRE(!matches(side, "sk"));
      REQUIRE(!matches(side, "buy"));
      REQUIRE(!matches(Side(), "sell"));
    }
  }

  TEST_CASE("match_quantity") {
    auto quantity = optional<Quantity>(1234.56);
    REQUIRE(matches(quantity, "1"));
    REQUIRE(matches(quantity, "12"));
    REQUIRE(matches(quantity, "34"));
    REQUIRE(matches(quantity, "1234"));
    REQUIRE(matches(quantity, "1234."));
    REQUIRE(matches(quantity, ".56"));
    REQUIRE(matches(quantity, "1234.56"));
    REQUIRE(!matches(quantity, "7"));
    REQUIRE(!matches(quantity, "60"));
    REQUIRE(!matches(quantity, "45"));
    REQUIRE(!matches(quantity, "123456"));
    REQUIRE(!matches(quantity, "0123"));
    REQUIRE(!matches(optional<Quantity>(), "1234.56"));
  }

  TEST_CASE("match_time_in_force") {
    auto time_in_force = TimeInForce(TimeInForce::Type::GTC);
    SUBCASE("match_time_in_force_display_name") {
      REQUIRE(matches(time_in_force, "g"));
      REQUIRE(matches(time_in_force, "Gt"));
      REQUIRE(matches(time_in_force, "tc"));
      REQUIRE(matches(time_in_force, "GTC"));
    }
    SUBCASE("match_time_in_force_description") {
      REQUIRE(matches(time_in_force, "go"));
      REQUIRE(matches(time_in_force, "Good"));
      REQUIRE(matches(time_in_force, "c"));
      REQUIRE(matches(time_in_force, "Can"));
      REQUIRE(matches(time_in_force, "cancel"));
      REQUIRE(matches(time_in_force, "good till"));
      REQUIRE(matches(time_in_force, "Good Till Cancel"));
    }
    SUBCASE("invalid_match") {
      REQUIRE(!matches(time_in_force, "d"));
      REQUIRE(!matches(time_in_force, "gc"));
      REQUIRE(!matches(time_in_force, "GTD"));
      REQUIRE(!matches(time_in_force, "god"));
      REQUIRE(!matches(time_in_force, "till"));
      REQUIRE(!matches(time_in_force, "good cancel"));
      REQUIRE(!matches(TimeInForce(), "GTC"));
    }
  }

  TEST_CASE("match_key") {
    auto key = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F8);
    REQUIRE(matches(key, "c"));
    REQUIRE(matches(key, "Ctrl"));
    REQUIRE(matches(key, "s"));
    REQUIRE(matches(key, "SHI"));
    REQUIRE(matches(key, "ft"));
    REQUIRE(matches(key, "shift"));
    REQUIRE(matches(key, "8"));
    REQUIRE(matches(key, "f8"));
    REQUIRE(matches(key, "shift f"));
    REQUIRE(matches(key, "ctrl Shift"));
    REQUIRE(matches(key, "ctrl Shift F8"));
    REQUIRE(!matches(key, "a"));
    REQUIRE(!matches(key, "ALT"));
    REQUIRE(!matches(key, "9"));
    REQUIRE(!matches(key, "CtrlShift"));
    REQUIRE(!matches(key, "CTRL+SHIFT"));
    REQUIRE(!matches(key, "Shift ctrl"));
    REQUIRE(!matches(key, "Shift 8"));
    REQUIRE(!matches(QKeySequence(), "F8"));
  }
}
