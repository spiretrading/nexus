#ifndef NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#define NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#include <memory>
#include <vector>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/QueuePipe.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <boost/optional.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /** Stores an ExecutionReport and the Order that published it. */
  struct ExecutionReportEntry {

    /** The Order that published the ExecutionReport. */
    std::shared_ptr<Order> m_order;

    /** The ExecutionReport that was published. */
    ExecutionReport m_report;

    /** Constructs an empty ExecutionReportEntry. */
    ExecutionReportEntry() noexcept;

    /**
     * Constructs an ExecutionReportEntry.
     * @param order The Order that published the ExecutionReport.
     * @param report The ExecutionReport that was published.
     */
    ExecutionReportEntry(
      std::shared_ptr<Order> order, ExecutionReport report) noexcept;
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
      explicit ExecutionReportPublisher(
        Beam::ScopedQueueReader<std::shared_ptr<Order>> orders);

      void With(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;

      void Monitor(Beam::ScopedQueueWriter<Type> monitor,
        Beam::Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(Beam::ScopedQueueWriter<ExecutionReportEntry> monitor)
        const override;

    private:
      Beam::SequencePublisher<ExecutionReportEntry> m_publisher;
      boost::optional<Beam::QueuePipe<std::shared_ptr<Order>>> m_pipe;
  };

  inline ExecutionReportEntry::ExecutionReportEntry() noexcept
    : m_order(nullptr) {}

  inline ExecutionReportEntry::ExecutionReportEntry(
    std::shared_ptr<Order> order, ExecutionReport report) noexcept
    : m_order(order),
      m_report(std::move(report)) {}

  inline ExecutionReportPublisher::ExecutionReportPublisher(
      Beam::ScopedQueueReader<std::shared_ptr<Order>> orders) {
    m_publisher.With([&] {
      auto snapshot = std::vector<ExecutionReportEntry>();
      while(auto order = orders.TryPop()) {
        auto reports = boost::optional<std::vector<ExecutionReport>>();
        (*order)->get_publisher().Monitor(
          Beam::MakeCallbackQueueWriter<ExecutionReport>(
            [=, this] (const auto& report) {
              m_publisher.Push(ExecutionReportEntry(*order, report));
            }), Beam::Store(reports));
        if(reports) {
          std::transform(
            reports->begin(), reports->end(), std::back_inserter(snapshot),
            [&] (const auto& report) {
              return ExecutionReportEntry(*order, report);
            });
        }
      }
      std::sort(snapshot.begin(), snapshot.end(),
        [] (const auto& lhs, const auto& rhs) {
          return std::tie(lhs.m_report.m_timestamp, lhs.m_report.m_id,
            lhs.m_report.m_status) < std::tie(rhs.m_report.m_timestamp,
              rhs.m_report.m_id, rhs.m_report.m_status);
        });
      for(auto& entry : snapshot) {
        m_publisher.Push(std::move(entry));
      }
    });
    m_pipe.emplace(std::move(orders),
      Beam::MakeCallbackQueueWriter<std::shared_ptr<Order>>(
        [this] (const auto& order) {
          order->get_publisher().Monitor(
            Beam::MakeCallbackQueueWriter<ExecutionReport>(
              [=, this] (const auto& report) {
                m_publisher.Push(ExecutionReportEntry(order, report));
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
