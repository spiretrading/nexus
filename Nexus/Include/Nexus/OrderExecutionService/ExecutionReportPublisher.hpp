#ifndef NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#define NEXUS_EXECUTION_REPORT_PUBLISHER_HPP
#include <memory>
#include <vector>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/QueuePipe.hpp>
#include <Beam/Queues/ScopedQueueGroup.hpp>
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

      void with(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;
      void monitor(Beam::ScopedQueueWriter<Type> monitor,
        Beam::Out<boost::optional<Snapshot>> snapshot) const override;
      void with(const std::function<void ()>& f) const override;
      void monitor(Beam::ScopedQueueWriter<ExecutionReportEntry> monitor)
        const override;

    private:
      Beam::SequencePublisher<ExecutionReportEntry> m_publisher;
      boost::optional<Beam::QueuePipe<std::shared_ptr<Order>>> m_pipe;
      Beam::ScopedQueueGroup m_callbacks;
  };

  inline ExecutionReportPublisher::ExecutionReportPublisher(
      Beam::ScopedQueueReader<std::shared_ptr<Order>> orders) {
    m_publisher.with([&] {
      auto snapshot = std::vector<ExecutionReportEntry>();
      while(auto order = orders.try_pop()) {
        auto reports = boost::optional<std::vector<ExecutionReport>>();
        (*order)->get_publisher().monitor(m_callbacks.add(
          Beam::callback<ExecutionReport>([=, this] (const auto& report) {
            m_publisher.push(ExecutionReportEntry(*order, report));
          })), Beam::out(reports));
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
        m_publisher.push(std::move(entry));
      }
    });
    m_pipe.emplace(std::move(orders), m_callbacks.add(
      Beam::callback<std::shared_ptr<Order>>([this] (const auto& order) {
        order->get_publisher().monitor(m_callbacks.add(
          Beam::callback<ExecutionReport>(
            [=, this] (const auto& report) {
              m_publisher.push(ExecutionReportEntry(order, report));
            })));
        })));
  }

  inline void ExecutionReportPublisher::with(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher.with(f);
  }

  inline void ExecutionReportPublisher::monitor(
      Beam::ScopedQueueWriter<Type> monitor,
      Beam::Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher.monitor(std::move(monitor), Beam::out(snapshot));
  }

  inline void ExecutionReportPublisher::with(
      const std::function<void ()>& f) const {
    m_publisher.with(f);
  }

  inline void ExecutionReportPublisher::monitor(
      Beam::ScopedQueueWriter<ExecutionReportEntry> monitor) const {
    m_publisher.monitor(std::move(monitor));
  }
}

#endif
