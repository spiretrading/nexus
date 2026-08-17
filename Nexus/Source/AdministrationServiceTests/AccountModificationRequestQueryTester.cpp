#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountModificationRequestQuery.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("AccountModificationRequestQuery") {
  TEST_CASE("make_account_modification_request_query") {
    auto account = DirectoryEntry::make_account(100, "account");
    auto query = make_account_modification_request_query(account, 25);
    REQUIRE(query.get_index() == account);
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::from_tail(25));
    REQUIRE(!query.get_anchor());
  }

  TEST_CASE("shuttle") {
    auto query = AccountModificationRequestQuery();
    query.set_index(DirectoryEntry::make_account(100, "account"));
    query.set_snapshot_limit(SnapshotLimit::from_head(25));
    query.set_anchor(AccountModificationRequestAnchor(42, ptime(), ""));
    query.set_filter(ConstantExpression(false));
    test_polymorphic_round_trip_shuttle(query, [] (auto registry) {
      register_query_types(registry);
    }, [&] (const auto& received) {
      REQUIRE(received.get_index() == query.get_index());
      REQUIRE(received.get_snapshot_limit() == query.get_snapshot_limit());
      REQUIRE(received.get_anchor() == query.get_anchor());
      REQUIRE(lexical_cast<std::string>(received.get_filter()) ==
        lexical_cast<std::string>(query.get_filter()));
    });
  }
}
