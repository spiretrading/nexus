#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("MarketDataRegistrySession") {
  TEST_CASE("has_entitlement_key") {
    auto session = MarketDataRegistrySession();
    auto key = EntitlementKey(TSX);
    REQUIRE(!has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles.Set(AccountRole::SERVICE);
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles = AccountRoles();
    session.m_roles.Set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      key, MarketDataTypeSet(MarketDataType::BBO_QUOTE));
    REQUIRE(has_entitlement(session, key, MarketDataType::BBO_QUOTE));
    REQUIRE(!has_entitlement(session, key, MarketDataType::BOOK_QUOTE));
    auto other_key = EntitlementKey(ASX);
    REQUIRE(!has_entitlement(session, other_key, MarketDataType::BBO_QUOTE));
  }

  TEST_CASE("has_entitlement_security_query") {
    auto session = MarketDataRegistrySession();
    auto security = Security("TST", TSX);
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::RealTime());
    REQUIRE(!has_entitlement<BboQuote>(session, query));
    session.m_roles.Set(AccountRole::SERVICE);
    REQUIRE(has_entitlement<BboQuote>(session, query));
    session.m_roles = AccountRoles();
    session.m_roles.Set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement<BboQuote>(session, query));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      EntitlementKey(TSX), MarketDataTypeSet(MarketDataType::BBO_QUOTE));
    REQUIRE(has_entitlement<BboQuote>(session, query));
    REQUIRE(!has_entitlement<BookQuote>(session, query));
    auto other_security = Security("S32", ASX);
    auto other_query = SecurityMarketDataQuery();
    other_query.SetIndex(other_security);
    other_query.SetRange(Range::RealTime());
    REQUIRE(!has_entitlement<BboQuote>(session, other_query));
  }

  TEST_CASE("has_entitlement_venue_query") {
    auto session = MarketDataRegistrySession();
    auto query = VenueMarketDataQuery();
    query.SetIndex(TSX);
    query.SetRange(Range::RealTime());
    REQUIRE(!has_entitlement<OrderImbalance>(session, query));
    session.m_roles.Set(AccountRole::SERVICE);
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    session.m_roles = AccountRoles();
    session.m_roles.Set(AccountRole::ADMINISTRATOR);
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    session.m_roles = AccountRoles();
    session.m_entitlements.grant(
      EntitlementKey(TSX), MarketDataTypeSet(MarketDataType::ORDER_IMBALANCE));
    REQUIRE(has_entitlement<OrderImbalance>(session, query));
    auto other_query = VenueMarketDataQuery();
    other_query.SetIndex(ASX);
    other_query.SetRange(Range::RealTime());
    REQUIRE(!has_entitlement<OrderImbalance>(session, other_query));
  }
}
