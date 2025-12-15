#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_TEST_SUITE_HPP
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"

namespace Nexus::Tests {
  inline void clear_time_in_force_timestamp(TimeInForce& time_in_force) {
    time_in_force = TimeInForce(time_in_force.get_type());
  }

  TEST_CASE_TEMPLATE_DEFINE(
      "OrderExecutionDataStore", T, OrderExecutionDataStoreTestSuite) {
    using namespace Beam;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus;
    using namespace Nexus::DefaultCurrencies;
    using namespace Nexus::DefaultVenues;

    auto data_store = T()();
    auto account_a = DirectoryEntry::make_account(123, "user_a");
    auto account_b = DirectoryEntry::make_account(456, "user_b");
    auto fields = make_limit_order_fields(
      account_a, Security("TST", TSX), CAD, Side::BID, "TSX", 100, Money::ONE);

    SUBCASE("store_and_load_order") {
      auto info = OrderInfo(fields, 1, time_from_string("2024-07-17 10:00:00"));
      auto sequenced_info = SequencedAccountOrderInfo(
        IndexedValue(info, account_a), Beam::Sequence(1));
      data_store.store(sequenced_info);
      auto loaded_record = data_store.load_order_record(1);
      clear_time_in_force_timestamp(
        (**loaded_record)->m_info.m_fields.m_time_in_force);
      REQUIRE(loaded_record);
      REQUIRE(**loaded_record == OrderRecord(info, {}));
      REQUIRE((*loaded_record)->get_index() == account_a);
      REQUIRE(loaded_record->get_sequence() == Beam::Sequence(1));
    }

    SUBCASE("load_non_existent_order") {
      auto loaded_record = data_store.load_order_record(999);
      REQUIRE(!loaded_record);
    }

    SUBCASE("store_and_load_multiple_orders") {
      auto info1 =
        OrderInfo(fields, 1, time_from_string("2024-07-17 10:01:00"));
      auto sequenced_info1 = SequencedAccountOrderInfo(
        IndexedValue(info1, account_a), Beam::Sequence(1));
      auto fields2 = make_limit_order_fields(account_b, Security("ABC", TSX),
        CAD, Side::ASK, "TSX", 200, 10 * Money::ONE);
      auto info2 =
        OrderInfo(fields2, 2, time_from_string("2024-07-17 10:02:00"));
      auto sequenced_info2 = SequencedAccountOrderInfo(
        IndexedValue(info2, account_b), Beam::Sequence(2));
      auto info3 =
        OrderInfo(fields, 3, time_from_string("2024-07-17 10:03:00"));
      auto sequenced_info3 = SequencedAccountOrderInfo(
        IndexedValue(info3, account_a), Beam::Sequence(3));
      data_store.store(
        std::vector{sequenced_info1, sequenced_info2, sequenced_info3});
      auto query_a = AccountQuery();
      query_a.set_index(account_a);
      query_a.set_range(Range::TOTAL);
      query_a.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_a = data_store.load_order_records(query_a);
      REQUIRE(results_a.size() == 2);
      clear_time_in_force_timestamp(
        results_a[0]->m_info.m_fields.m_time_in_force);
      REQUIRE(results_a[0] ==
        SequencedValue(OrderRecord(info1, {}), Beam::Sequence(1)));
      clear_time_in_force_timestamp(
        results_a[1]->m_info.m_fields.m_time_in_force);
      REQUIRE(results_a[1] ==
        SequencedValue(OrderRecord(info3, {}), Beam::Sequence(3)));
      auto query_b = AccountQuery();
      query_b.set_index(account_b);
      query_b.set_range(Range::TOTAL);
      query_b.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_b = data_store.load_order_records(query_b);
      REQUIRE(results_b.size() == 1);
      clear_time_in_force_timestamp(
        results_b[0]->m_info.m_fields.m_time_in_force);
      REQUIRE(results_b[0] ==
        SequencedValue(OrderRecord(info2, {}), Beam::Sequence(2)));
    }

    SUBCASE("store_and_load_execution_reports") {
      auto info = OrderInfo(fields, 1, time_from_string("2024-07-17 10:05:00"));
      auto sequenced_info = SequencedAccountOrderInfo(
        IndexedValue(info, account_a), Beam::Sequence(1));
      data_store.store(sequenced_info);
      auto report1 =
        ExecutionReport(1, time_from_string("2024-07-17 10:05:00"));
      report1.m_status = OrderStatus::PENDING_NEW;
      report1.m_sequence = 0;
      auto sequenced_report1 = SequencedAccountExecutionReport(
        IndexedValue(report1, account_a), Beam::Sequence(2));
      data_store.store(sequenced_report1);
      auto report2 = make_update(
        report1, OrderStatus::NEW, time_from_string("2024-07-17 10:06:00"));
      auto report3 = make_update(
        report2, OrderStatus::FILLED, time_from_string("2024-07-17 10:07:00"));
      auto sequenced_report2 = SequencedAccountExecutionReport(
        IndexedValue(report2, account_a), Beam::Sequence(3));
      auto sequenced_report3 = SequencedAccountExecutionReport(
        IndexedValue(report3, account_a), Beam::Sequence(4));
      data_store.store(std::vector{sequenced_report2, sequenced_report3});
      auto query = AccountQuery();
      query.set_index(account_a);
      query.set_range(Range::TOTAL);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results = data_store.load_execution_reports(query);
      REQUIRE(results.size() == 3);
      REQUIRE(results[0] == SequencedValue(report1, Beam::Sequence(2)));
      REQUIRE(results[1] == SequencedValue(report2, Beam::Sequence(3)));
      REQUIRE(results[2] == SequencedValue(report3, Beam::Sequence(4)));
    }

    SUBCASE("load_order_with_execution_reports") {
      auto info = OrderInfo(fields, 1, time_from_string("2024-07-17 10:10:00"));
      auto sequenced_info = SequencedAccountOrderInfo(
        IndexedValue(info, account_a), Beam::Sequence(1));
      data_store.store(sequenced_info);
      auto report1 =
        ExecutionReport(1, time_from_string("2024-07-17 10:10:00"));
      report1.m_status = OrderStatus::PENDING_NEW;
      report1.m_sequence = 0;
      auto sequenced_report1 = SequencedAccountExecutionReport(
        IndexedValue(report1, account_a), Beam::Sequence(2));
      data_store.store(sequenced_report1);
      auto report2 = make_update(
        report1, OrderStatus::NEW, time_from_string("2024-07-17 10:11:00"));
      auto sequenced_report2 = SequencedAccountExecutionReport(
        IndexedValue(report2, account_a), Beam::Sequence(3));
      data_store.store(sequenced_report2);
      auto loaded_record = data_store.load_order_record(1);
      REQUIRE(loaded_record);
      clear_time_in_force_timestamp(
        (**loaded_record)->m_info.m_fields.m_time_in_force);
      REQUIRE((**loaded_record)->m_info == info);
      REQUIRE((**loaded_record)->m_execution_reports.size() == 2);
      REQUIRE((**loaded_record)->m_execution_reports[0] == report1);
      REQUIRE((**loaded_record)->m_execution_reports[1] == report2);
    }
  }
}

#endif
