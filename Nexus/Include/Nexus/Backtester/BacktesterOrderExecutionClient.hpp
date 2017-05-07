#ifndef NEXUS_BACKTESTERORDEREXECUTIONCLIENT_HPP
#define NEXUS_BACKTESTERORDEREXECUTIONCLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

namespace Nexus {

  /*! \class BacktesterOrderExecutionClient
      \brief Implements an OrderExecutionClient used for backtesting.
   */
  class BacktesterOrderExecutionClient : private boost::noncopyable {
    public:

      //! Constructs a BacktesterOrderExecutionClient.
      /*!
        \param environment The BacktesterEnvironment to connect to.
        \param orderExecutionClient The underlying OrderExecutionClient to
               submit Orders to.
      */
      BacktesterOrderExecutionClient(
        Beam::RefType<BacktesterEnvironment> environment,
        std::unique_ptr<OrderExecutionService::VirtualOrderExecutionClient>
        orderExecutionClient);

      ~BacktesterOrderExecutionClient();

      void QueryOrderRecords(const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        OrderExecutionService::OrderRecord>>& queue);

      void QueryOrderSubmissions(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        OrderExecutionService::SequencedOrder>>& queue);

      void QueryOrderSubmissions(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        const OrderExecutionService::Order*>>& queue);

      void QueryExecutionReports(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<
        OrderExecutionService::ExecutionReport>>& queue);

      const OrderExecutionService::OrderExecutionPublisher&
        GetOrderSubmissionPublisher();

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderFields& fields);

      void Cancel(const OrderExecutionService::Order& order);

      void Open();

      void Close();

    private:
      BacktesterEnvironment* m_environment;
      std::unique_ptr<OrderExecutionService::VirtualOrderExecutionClient>
        m_orderExecutionClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterOrderExecutionClient::BacktesterOrderExecutionClient(
      Beam::RefType<BacktesterEnvironment> environment,
      std::unique_ptr<OrderExecutionService::VirtualOrderExecutionClient>
      orderExecutionClient)
      : m_environment{environment.Get()},
        m_orderExecutionClient{std::move(orderExecutionClient)} {}

  inline BacktesterOrderExecutionClient::~BacktesterOrderExecutionClient() {
    Close();
  }

  inline void BacktesterOrderExecutionClient::QueryOrderRecords(
      const OrderExecutionService::AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<
      OrderExecutionService::OrderRecord>>& queue) {
    return m_orderExecutionClient->QueryOrderRecords(query, queue);
  }

  inline void BacktesterOrderExecutionClient::QueryOrderSubmissions(
      const OrderExecutionService::AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<
      OrderExecutionService::SequencedOrder>>& queue) {
    return m_orderExecutionClient->QueryOrderSubmissions(query, queue);
  }

  inline void BacktesterOrderExecutionClient::QueryOrderSubmissions(
      const OrderExecutionService::AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<
      const OrderExecutionService::Order*>>& queue) {
    return m_orderExecutionClient->QueryOrderSubmissions(query, queue);
  }

  inline void BacktesterOrderExecutionClient::QueryExecutionReports(
      const OrderExecutionService::AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<
      OrderExecutionService::ExecutionReport>>& queue) {
    return m_orderExecutionClient->QueryExecutionReports(query, queue);
  }

  inline const OrderExecutionService::OrderExecutionPublisher&
      BacktesterOrderExecutionClient::GetOrderSubmissionPublisher() {
    return m_orderExecutionClient->GetOrderSubmissionPublisher();
  }

  inline const OrderExecutionService::Order&
      BacktesterOrderExecutionClient::Submit(
      const OrderExecutionService::OrderFields& fields) {
    return m_orderExecutionClient->Submit(fields);
  }

  inline void BacktesterOrderExecutionClient::Cancel(
      const OrderExecutionService::Order& order) {
    m_orderExecutionClient->Cancel(order);
  }

  inline void BacktesterOrderExecutionClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_orderExecutionClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterOrderExecutionClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterOrderExecutionClient::Shutdown() {
    m_orderExecutionClient->Close();
    m_openState.SetClosed();
  }
}

#endif
