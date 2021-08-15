#ifndef NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#define NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#include <vector>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/QueuePipe.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <boost/optional.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Stores an ExecutionReport and the Order that published it. */
  struct ExecutionReportEntry {

    /** The Order that published the ExecutionReport. */
    const Order* m_order;

    /** The ExecutionReport that was published. */
    ExecutionReport m_executionReport;

    /** Constructs an empty ExecutionReportEntry. */
    ExecutionReportEntry();

    /**
     * Constructs an ExecutionReportEntry.
     * @param order The Order that published the ExecutionReport.
     * @param executionReport The ExecutionReport that was published.
     */
    ExecutionReportEntry(const Order* order,
      ExecutionReport executionReport);
  };

  /**
   * Publishes a synchronized sequence of ExecutionReportEntries based off of
   * the Orders published by an OrderExecutionPublisher.
   */
  class ExecutionReportPublisher final : public Beam::SnapshotPublisher<
      ExecutionReportEntry, std::vector<ExecutionReportEntry>> {
    public:

      /**
       * Constructs an ExecutionReportPublisher.
       * @param orders The Orders whose ExecutionReports are to be published.
       */
      ExecutionReportPublisher(Beam::ScopedQueueReader<const Order*> orders);

      void With(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;

      void Monitor(Beam::ScopedQueueWriter<Type> monitor,
        Beam::Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(Beam::ScopedQueueWriter<ExecutionReportEntry> monitor)
        const override;

    private:
      Beam::SequencePublisher<ExecutionReportEntry> m_publisher;
      boost::optional<Beam::QueuePipe<const Order*>> m_pipe;
  };

  inline ExecutionReportEntry::ExecutionReportEntry()
    : m_order(nullptr) {}

  inline ExecutionReportEntry::ExecutionReportEntry(const Order* order,
    ExecutionReport executionReport)
    : m_order(order),
      m_executionReport(std::move(executionReport)) {}

  inline ExecutionReportPublisher::ExecutionReportPublisher(
      Beam::ScopedQueueReader<const Order*> orders) {
    m_publisher.With([&] {
      auto executionReportEntrySnapshot = std::vector<ExecutionReportEntry>();
      while(auto topOrder = orders.TryPop()) {
        auto order = *topOrder;
        auto executionReportSnapshot =
          boost::optional<std::vector<ExecutionReport>>();
        order->GetPublisher().Monitor(
          Beam::MakeCallbackQueueWriter<ExecutionReport>(
            [=, this] (const auto& executionReport) {
              m_publisher.Push(ExecutionReportEntry(order, executionReport));
            }), Beam::Store(executionReportSnapshot));
        if(executionReportSnapshot) {
          std::transform(executionReportSnapshot->begin(),
            executionReportSnapshot->end(),
            std::back_inserter(executionReportEntrySnapshot),
            [&] (auto& executionReport) {
              return ExecutionReportEntry(order, executionReport);
            });
        }
      }
      std::sort(executionReportEntrySnapshot.begin(),
        executionReportEntrySnapshot.end(),
        [] (auto& lhs, auto& rhs) {
          return std::tie(lhs.m_executionReport.m_timestamp,
            lhs.m_executionReport.m_id, lhs.m_executionReport.m_status) <
            std::tie(rhs.m_executionReport.m_timestamp,
            rhs.m_executionReport.m_id, rhs.m_executionReport.m_status);
        });
      for(auto& executionReportEntry : executionReportEntrySnapshot) {
        m_publisher.Push(std::move(executionReportEntry));
      }
    });
    m_pipe.emplace(std::move(orders),
      Beam::MakeCallbackQueueWriter<const Order*>([this] (auto order) {
        order->GetPublisher().Monitor(
          Beam::MakeCallbackQueueWriter<ExecutionReport>(
          [=, this] (const auto& executionReport) {
            m_publisher.Push(ExecutionReportEntry(order, executionReport));
          }));
      }));
  }

  inline void ExecutionReportPublisher::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher.With(f);
  }

  inline void ExecutionReportPublisher::Monitor(
      Beam::ScopedQueueWriter<Type> monitor,
      Beam::Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher.Monitor(std::move(monitor), Beam::Store(snapshot));
  }

  inline void ExecutionReportPublisher::With(
      const std::function<void ()>& f) const {
    m_publisher.With(f);
  }

  inline void ExecutionReportPublisher::Monitor(
      Beam::ScopedQueueWriter<ExecutionReportEntry> monitor) const {
    m_publisher.Monitor(std::move(monitor));
  }
}

#endif
