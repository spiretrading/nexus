#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderImbalance.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OrderImbalance") {
  TEST_CASE("stream") {
    auto imbalance = OrderImbalance(Security("ABC", DefaultVenues::TSX),
      Side::BID, 42, Money(12345), time_from_string("2020-01-02 03:04:05"));
    auto ss = std::ostringstream();
    ss << imbalance.m_security;
    auto expected_security = ss.str();
    ss.str("");
    ss.clear();
    ss << imbalance.m_side;
    auto expected_side = ss.str();
    ss.str("");
    ss.clear();
    ss << imbalance.m_size;
    auto expected_size = ss.str();
    ss.str("");
    ss.clear();
    ss << imbalance.m_reference_price;
    auto expected_price = ss.str();
    ss.str("");
    ss.clear();
    ss << imbalance.m_timestamp;
    auto expected_timestamp = ss.str();
    ss = std::ostringstream();
    ss << imbalance;
    auto expected_output = "(" + expected_security + " " + expected_side +
      " " + expected_size + " " + expected_price + " " + expected_timestamp +
      ")";
    CHECK(ss.str() == expected_output);
  }
}
