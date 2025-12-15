#ifndef NEXUS_LOCAL_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_LOCAL_ORDER_EXECUTION_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Queries/LocalDataStore.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {

  /** Stores an in memory database of Order execution data. */
  class LocalOrderExecutionDataStore {
    public:

      /** Constructs an empty LocalOrderExecutionDataStore. */
      LocalOrderExecutionDataStore();

      /** Returns all Order submissions stored. */
      std::vector<SequencedAccountOrderRecord> load_order_submissions() const;

      /** Returns all the ExecutionReports stored. */
      std::vector<SequencedAccountExecutionReport>
        load_execution_reports() const;

      boost::optional<SequencedAccountOrderRecord>
        load_order_record(OrderId id);
      std::vector<SequencedOrderRecord>
        load_order_records(const AccountQuery& query);
      void store(const SequencedAccountOrderInfo& info);
      void store(const std::vector<SequencedAccountOrderInfo>& info);
      std::vector<SequencedExecutionReport>
        load_execution_reports(const AccountQuery& query);
      void store(const SequencedAccountExecutionReport& report);
      void store(const std::vector<SequencedAccountExecutionReport>& reports);
      void close();

    private:
      template<typename T>
      using DataStore =
        Beam::LocalDataStore<AccountQuery, T, EvaluatorTranslator>;
      Beam::SynchronizedUnorderedSet<OrderId> m_live_orders;
      DataStore<OrderInfo> m_order_submission_data_store;
      DataStore<ExecutionReport> m_execution_report_data_store;
      Beam::SynchronizedUnorderedMap<OrderId, SequencedAccountOrderInfo>
        m_orders;
      mutable Beam::SynchronizedUnorderedMap<
        OrderId, Beam::SynchronizedVector<ExecutionReport>> m_execution_reports;
  };

  inline LocalOrderExecutionDataStore::LocalOrderExecutionDataStore()
    : m_order_submission_data_store(Beam::Ref(m_live_orders)) {}

  inline std::vector<SequencedAccountOrderRecord>
      LocalOrderExecutionDataStore::load_order_submissions() const {
    auto submissions = m_order_submission_data_store.load_all();
    auto records = std::vector<SequencedAccountOrderRecord>();
    for(auto& submission : submissions) {
      auto record = OrderRecord(
        **submission, m_execution_reports.get((*submission)->m_id).load());
      records.emplace_back(Beam::IndexedValue(record, submission->get_index()),
        submission.get_sequence());
    }
    return records;
  }

  inline std::vector<SequencedAccountExecutionReport>
      LocalOrderExecutionDataStore::load_execution_reports() const {
    return m_execution_report_data_store.load_all();
  }

  inline boost::optional<SequencedAccountOrderRecord>
      LocalOrderExecutionDataStore::load_order_record(OrderId id) {
    auto info = m_orders.try_load(id);
    if(!info) {
      return boost::none;
    }
    return Beam::SequencedValue(Beam::IndexedValue(
      OrderRecord(***info, m_execution_reports.get(id).load()),
      (*info)->get_index()), info->get_sequence());
  }

  inline std::vector<SequencedOrderRecord>
      LocalOrderExecutionDataStore::load_order_records(
        const AccountQuery& query) {
    auto submissions = m_order_submission_data_store.load(query);
    auto records = std::vector<SequencedOrderRecord>();
    for(auto& submission : submissions) {
      auto record = OrderRecord(
        *submission, m_execution_reports.get(submission->m_id).load());
      records.emplace_back(record, submission.get_sequence());
    }
    return records;
  }

  inline void LocalOrderExecutionDataStore::store(
      const SequencedAccountOrderInfo& info) {
    m_order_submission_data_store.store(info);
    m_orders.insert((*info)->m_id, info);
    m_live_orders.insert((*info)->m_id);
  }

  inline void LocalOrderExecutionDataStore::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    m_order_submission_data_store.store(info);
    for(auto& i : info) {
      m_live_orders.insert((*i)->m_id);
      m_orders.insert((*i)->m_id, i);
    }
  }

  inline std::vector<SequencedExecutionReport>
      LocalOrderExecutionDataStore::load_execution_reports(
        const AccountQuery& query) {
    return m_execution_report_data_store.load(query);
  }

  inline void LocalOrderExecutionDataStore::store(
      const SequencedAccountExecutionReport& report) {
    m_execution_report_data_store.store(report);
    m_execution_reports.get((*report)->m_id).with([&] (auto& reports) {
      if(reports.empty() || (*report)->m_sequence > reports.back().m_sequence) {
        reports.push_back(**report);
        return;
      }
      auto insertion_point =
        std::lower_bound(reports.begin(), reports.end(), **report,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_sequence < rhs.m_sequence;
          });
      if(insertion_point->m_sequence == (*report)->m_sequence) {
        return;
      }
      reports.insert(insertion_point, **report);
    });
    if(is_terminal((*report)->m_status)) {
      m_live_orders.erase((*report)->m_id);
    }
  }

  inline void LocalOrderExecutionDataStore::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    for(auto& report : reports) {
      store(report);
    }
  }

  inline void LocalOrderExecutionDataStore::close() {}
}

#endif
