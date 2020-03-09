#ifndef NEXUS_EXECUTIONREPORTPUBLISHER_HPP
#define NEXUS_EXECUTIONREPORTPUBLISHER_HPP
#include <vector>
#include <Beam/Queues/CallbackWriterQueue.hpp>
#include <Beam/Queues/ConverterWriterQueue.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \struct ExecutionReportEntry
      \brief Stores an ExecutionReport and the Order that published it.
   */
  struct ExecutionReportEntry {

    //! The Order that published the ExecutionReport.
    const Order* m_order;

    //! The ExecutionReport that was published.
    ExecutionReport m_executionReport;

    //! Constructs an empty ExecutionReportEntry.
    ExecutionReportEntry();

    //! Constructs an ExecutionReportEntry.
    /*!
      \param order The Order that published the ExecutionReport.
      \param executionReport The ExecutionReport that was published.
    */
    ExecutionReportEntry(const Order* order,
      const ExecutionReport& executionReport);
  };

  /*! \class ExecutionReportPublisher
      \brief Publishes a synchronized sequence of ExecutionReportEntries based
             off of the Orders published by an OrderExecutionPublisher.
   */
  class ExecutionReportPublisher final :
      public Beam::Publisher<ExecutionReportEntry> {
    public:

      //! Constructs an ExecutionReportPublisher.
      /*!
        \param orderExecutionPublisher The OrderExecutionPublisher to monitor
               for Orders.
      */
      ExecutionReportPublisher(
        const OrderExecutionPublisher& orderExecutionPublisher);

      void With(const std::function<void ()>& f) const override;

      void Monitor(
        std::shared_ptr<Beam::QueueWriter<ExecutionReportEntry>> monitor)
        const override;

    private:
      std::shared_ptr<Beam::SequencePublisher<ExecutionReportEntry>>
        m_publisher;
      std::shared_ptr<Beam::CallbackWriterQueue<const Order*>> m_callbackQueue;
      std::vector<std::shared_ptr<Beam::QueueWriter<ExecutionReport>>>
        m_converters;
  };

  inline ExecutionReportEntry::ExecutionReportEntry()
    : m_order(nullptr) {}

  inline ExecutionReportEntry::ExecutionReportEntry(const Order* order,
    const ExecutionReport& executionReport)
    : m_order(order),
      m_executionReport(executionReport) {}

  inline ExecutionReportPublisher::ExecutionReportPublisher(
      const OrderExecutionPublisher& orderExecutionPublisher)
      BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_publisher(std::make_shared<
          Beam::SequencePublisher<ExecutionReportEntry>>()),
        m_callbackQueue(
          std::make_shared<Beam::CallbackWriterQueue<const Order*>>(
          [=] (const Order* order) {
            std::shared_ptr<Beam::QueueWriter<ExecutionReport>> converter =
              std::make_shared<Beam::CallbackWriterQueue<ExecutionReport>>(
              [=] (const ExecutionReport& executionReport) {
                m_publisher->Push(ExecutionReportEntry(order, executionReport));
              });
            m_converters.push_back(converter);
            order->GetPublisher().Monitor(converter);
          })) {
      BEAM_UNSUPPRESS_THIS_INITIALIZER()
    orderExecutionPublisher.With(
      [&] {
        boost::optional<std::vector<const Order*>> orderSnapshot;
        orderExecutionPublisher.Monitor(m_callbackQueue,
          Beam::Store(orderSnapshot));
        if(!orderSnapshot.is_initialized()) {
          return;
        }
        try {
          std::vector<ExecutionReportEntry> executionReportEntrySnapshot;
          for(const Order* order : *orderSnapshot) {
            order->GetPublisher().With(
              [&] {
                std::shared_ptr<Beam::QueueWriter<ExecutionReport>> converter =
                  std::make_shared<Beam::CallbackWriterQueue<ExecutionReport>>(
                  [=] (const ExecutionReport& executionReport) {
                    m_publisher->Push(ExecutionReportEntry(order,
                      executionReport));
                  });
                boost::optional<std::vector<ExecutionReport>>
                  executionReportSnapshot;
                order->GetPublisher().Monitor(converter,
                  Beam::Store(executionReportSnapshot));
                if(!executionReportSnapshot.is_initialized()) {
                  return;
                }
                std::transform(executionReportSnapshot->begin(),
                  executionReportSnapshot->end(),
                  std::back_inserter(executionReportEntrySnapshot),
                  [&] (const ExecutionReport& executionReport) {
                    return ExecutionReportEntry(order, executionReport);
                  });
              });
          }
          std::sort(executionReportEntrySnapshot.begin(),
            executionReportEntrySnapshot.end(),
            [] (const ExecutionReportEntry& lhs,
                const ExecutionReportEntry& rhs) {
            return std::tie(lhs.m_executionReport.m_timestamp,
              lhs.m_executionReport.m_id, lhs.m_executionReport.m_status) <
              std::tie(rhs.m_executionReport.m_timestamp,
              rhs.m_executionReport.m_id, rhs.m_executionReport.m_status);
            });
          for(const ExecutionReportEntry& executionReportEntry :
              executionReportEntrySnapshot) {
            m_publisher->Push(executionReportEntry);
          }
        } catch(...) {}
      });
  }

  inline void ExecutionReportPublisher::With(
      const std::function<void ()>& f) const {
    m_publisher->With(f);
  }

  inline void ExecutionReportPublisher::Monitor(
      std::shared_ptr<Beam::QueueWriter<ExecutionReportEntry>> monitor) const {
    m_publisher->Monitor(monitor);
  }
}
}

#endif
