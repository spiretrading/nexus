#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("MarketDataRegistrySession") {
  TEST_CASE("has_entitlement_key") {
    auto session = MarketDataRegistrySession();
    auto key = EntitlementKey(TSX);
    REQUIRE(!has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles.set(AccountRole::SERVICE);
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles = AccountRoles();
    session.m_roles.set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      key, MarketDataTypeSet(MarketDataType::BBO_QUOTE));
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    REQUIRE(!has_entitlement(session, key, MarketDataType::BOOK_QUOTE));
    auto other_key = EntitlementKey(ASX);
    REQUIRE(!has_entitlement(session, other_key, MarketDataType::BBO_QUOTE));
  }

  TEST_CASE("has_entitlement_ticker_query") {
    auto session = MarketDataRegistrySession();
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerMarketDataQuery();
    query.set_index(ticker);
    query.set_range(Range::REAL_TIME);
    REQUIRE(!has_entitlement<BboQuote>(session, query));
    session.m_roles.set(AccountRole::SERVICE);
    REQUIRE(has_entitlement<BboQuote>(session, query));
    session.m_roles = AccountRoles();
    session.m_roles.set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement<BboQuote>(session, query));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      EntitlementKey(TSX), MarketDataTypeSet(MarketDataType::BBO_QUOTE));
    REQUIRE(has_entitlement<BboQuote>(session, query));
    REQUIRE(!has_entitlement<BookQuote>(session, query));
    auto other_ticker = parse_ticker("S32.ASX");
    auto other_query = TickerMarketDataQuery();
    other_query.set_index(other_ticker);
    other_query.set_range(Range::REAL_TIME);
    REQUIRE(!has_entitlement<BboQuote>(session, other_query));
  }

  TEST_CASE("has_entitlement_venue_query") {
    auto session = MarketDataRegistrySession();
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    query.set_range(Range::REAL_TIME);
    REQUIRE(!has_entitlement<OrderImbalance>(session, query));
    session.m_roles.set(AccountRole::SERVICE);
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    session.m_roles = AccountRoles();
    session.m_roles.set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      EntitlementKey(TSX), MarketDataTypeSet(MarketDataType::ORDER_IMBALANCE));
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    auto other_query = VenueMarketDataQuery();
    other_query.set_index(ASX);
    other_query.set_range(Range::REAL_TIME);
    REQUIRE(!has_entitlement<OrderImbalance>(session, other_query));
  }
}
