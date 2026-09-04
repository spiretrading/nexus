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
    query.set_anchor(AccountModificationRequestAnchor(
      42, time_from_string("2024-07-04 12:00:00"), "alpha"));
    query.set_offset(3);
    query.set_filter(ConstantExpression(false));
    query.set_sort_field(
      AccountModificationRequestQuery::SortField::EFFECTIVE_DATE);
    query.set_search("beta");
    test_polymorphic_round_trip_shuttle(query, [] (auto registry) {
      register_query_types(registry);
    }, [&] (const auto& received) {
      REQUIRE(received.get_index() == query.get_index());
      REQUIRE(received.get_snapshot_limit() == query.get_snapshot_limit());
      REQUIRE(received.get_anchor() == query.get_anchor());
      REQUIRE(received.get_offset() == query.get_offset());
      REQUIRE(received.get_sort_field() == query.get_sort_field());
      REQUIRE(received.get_search() == query.get_search());
      REQUIRE(lexical_cast<std::string>(received.get_filter()) ==
        lexical_cast<std::string>(query.get_filter()));
    });
  }

  TEST_CASE("shuttle_clamps_an_unknown_sort_field") {
    auto query = AccountModificationRequestQuery();
    query.set_sort_field(
      static_cast<AccountModificationRequestQuery::SortField>(99));
    test_polymorphic_round_trip_shuttle(query, [] (auto registry) {
      register_query_types(registry);
    }, [&] (const auto& received) {
      REQUIRE(received.get_sort_field() ==
        AccountModificationRequestQuery::SortField::CREATED);
    });
  }

  TEST_CASE("sort_field_stream") {
    REQUIRE(lexical_cast<std::string>(
      AccountModificationRequestQuery::SortField::CREATED) == "CREATED");
    REQUIRE(lexical_cast<std::string>(
      AccountModificationRequestQuery::SortField::LAST_UPDATED) ==
        "LAST_UPDATED");
    REQUIRE(lexical_cast<std::string>(
      AccountModificationRequestQuery::SortField::EFFECTIVE_DATE) ==
        "EFFECTIVE_DATE");
    REQUIRE(lexical_cast<std::string>(
      AccountModificationRequestQuery::SortField::ACCOUNT) == "ACCOUNT");
    REQUIRE(lexical_cast<std::string>(
      AccountModificationRequestQuery::SortField::REQUESTER) == "REQUESTER");
    REQUIRE(lexical_cast<std::string>(
      static_cast<AccountModificationRequestQuery::SortField>(99)) ==
        "CREATED");
  }

  TEST_CASE("anchor_stream") {
    REQUIRE(lexical_cast<std::string>(AccountModificationRequestAnchor(
      42, time_from_string("2024-07-04 12:00:00"), "alpha")) ==
        "(42 2024-Jul-04 12:00:00 alpha)");
    REQUIRE(lexical_cast<std::string>(AccountModificationRequestAnchor()) ==
      "(-1 -infinity )");
  }

  TEST_CASE("query_stream") {
    auto query = AccountModificationRequestQuery();
    query.set_index(DirectoryEntry::make_account(100, "account"));
    query.set_snapshot_limit(SnapshotLimit::from_head(25));
    query.set_sort_field(
      AccountModificationRequestQuery::SortField::LAST_UPDATED);
    REQUIRE(lexical_cast<std::string>(query) ==
      "(((ACCOUNT 100 account) (HEAD 25) true) LAST_UPDATED)");
    query.set_anchor(AccountModificationRequestAnchor(
      42, time_from_string("2024-07-04 12:00:00"), "alpha"));
    query.set_offset(3);
    REQUIRE(
      lexical_cast<std::string>(query) == "(((ACCOUNT 100 account) (HEAD 25) "
      "(42 2024-Jul-04 12:00:00 alpha) 3 true) LAST_UPDATED)");
  }
}
