#ifndef NEXUS_LOCALORDEREXECUTIONDATASTORE_HPP
#define NEXUS_LOCALORDEREXECUTIONDATASTORE_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Queries/LocalDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class LocalOrderExecutionDataStore
      \brief Stores an in memory database of Order execution data.
   */
  class LocalOrderExecutionDataStore : private boost::noncopyable {
    public:

      //! Constructs an empty LocalOrderExecutionDataStore.
      LocalOrderExecutionDataStore();

      //! Returns all Order submissions stored.
      std::vector<SequencedAccountOrderRecord> LoadOrderSubmissions() const;

      //! Returns all the ExecutionReports stored.
      std::vector<SequencedAccountExecutionReport> LoadExecutionReports() const;

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const std::vector<SequencedAccountOrderInfo>& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Store(const std::vector<SequencedAccountExecutionReport>&
        executionReports);

      void Open();

      void Close();

    private:
      template<typename T>
      using DataStore = Beam::Queries::LocalDataStore<AccountQuery, T,
        Queries::EvaluatorTranslator>;
      Beam::SynchronizedUnorderedSet<OrderId> m_liveOrders;
      DataStore<OrderInfo> m_orderSubmissionDataStore;
      DataStore<ExecutionReport> m_executionReportDataStore;
      mutable Beam::SynchronizedUnorderedMap<
        OrderId, Beam::SynchronizedVector<ExecutionReport>> m_executionReports;
  };

  inline LocalOrderExecutionDataStore::LocalOrderExecutionDataStore()
      : m_orderSubmissionDataStore{Beam::Ref(m_liveOrders)} {}

  inline std::vector<SequencedAccountOrderRecord> LocalOrderExecutionDataStore::
      LoadOrderSubmissions() const {
    auto submissions = m_orderSubmissionDataStore.LoadAll();
    std::vector<SequencedAccountOrderRecord> orderRecords;
    for(auto& submission : submissions) {
      OrderRecord orderRecord{*submission,
        m_executionReports.Get((*submission)->m_orderId).Acquire()};
      orderRecords.emplace_back(
        Beam::Queries::IndexedValue(orderRecord, submission->GetIndex()),
        submission.GetSequence());
    }
    return orderRecords;
  }

  inline std::vector<SequencedAccountExecutionReport>
      LocalOrderExecutionDataStore::LoadExecutionReports() const {
    return m_executionReportDataStore.LoadAll();
  }

  inline std::vector<SequencedOrderRecord>
      LocalOrderExecutionDataStore::LoadOrderSubmissions(
      const AccountQuery& query) {
    auto submissions = m_orderSubmissionDataStore.Load(query);
    std::vector<SequencedOrderRecord> orderRecords;
    for(auto& submission : submissions) {
      OrderRecord orderRecord{*submission,
        m_executionReports.Get(submission->m_orderId).Acquire()};
      orderRecords.emplace_back(orderRecord, submission.GetSequence());
    }
    return orderRecords;
  }

  inline std::vector<SequencedExecutionReport>
      LocalOrderExecutionDataStore::LoadExecutionReports(
      const AccountQuery& query) {
    return m_executionReportDataStore.Load(query);
  }

  inline void LocalOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_orderSubmissionDataStore.Store(orderInfo);
    m_liveOrders.Insert((*orderInfo)->m_orderId);
  }

  inline void LocalOrderExecutionDataStore::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    m_orderSubmissionDataStore.Store(orderInfo);
    for(auto& info : orderInfo) {
      m_liveOrders.Insert((*info)->m_orderId);
    }
  }

  inline void LocalOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_executionReportDataStore.Store(executionReport);
    m_executionReports.Get((*executionReport)->m_id).With(
      [&] (std::vector<ExecutionReport>& executionReports) {
        if(executionReports.empty() || (*executionReport)->m_sequence >
            executionReports.back().m_sequence) {
          executionReports.push_back(**executionReport);
          return;
        }
        auto insertionPoint = Beam::LinearLowerBound(executionReports.begin(),
          executionReports.end(), **executionReport,
          [] (const ExecutionReport& lhs, const ExecutionReport& rhs) {
            return lhs.m_sequence < rhs.m_sequence;
          });
        if(insertionPoint->m_sequence == (*executionReport)->m_sequence) {
          return;
        }
        executionReports.insert(insertionPoint, **executionReport);
      });
    if(IsTerminal((*executionReport)->m_status)) {
      m_liveOrders.Erase((*executionReport)->m_id);
    }
  }

  inline void LocalOrderExecutionDataStore::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    for(auto& executionReport : executionReports) {
      Store(executionReport);
    }
  }

  inline void LocalOrderExecutionDataStore::Open() {}

  inline void LocalOrderExecutionDataStore::Close() {}
}
}

#endif
