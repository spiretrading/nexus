#ifndef NEXUS_TO_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ORDER_EXECUTION_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class ToPythonOrderExecutionClient
      \brief Wraps an OrderExecutionClient for use with Python.
      \tparam ClientType The type of OrderExecutionClient to wrap.
   */
  template<typename ClientType>
  class ToPythonOrderExecutionClient : public VirtualOrderExecutionClient {
    public:

      //! The type of OrderExecutionClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonOrderExecutionClient.
      /*!
        \param client The OrderExecutionClient to wrap.
      */
      ToPythonOrderExecutionClient(std::unique_ptr<Client> client);

      virtual ~ToPythonOrderExecutionClient() override final;

      virtual void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue)
        override final;

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue)
        override final;

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue)
        override final;

      virtual void QueryExecutionReports(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue)
        override final;

      virtual const OrderExecutionPublisher& GetOrderSubmissionPublisher()
      override final;

      virtual const Order& Submit(const OrderFields& fields) override final;

      virtual void Cancel(const Order& order) override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
  };

  //! Makes a ToPythonOrderExecutionClient.
  /*!
    \param client The OrderExecutionClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonOrderExecutionClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonOrderExecutionClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonOrderExecutionClient<ClientType>::ToPythonOrderExecutionClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonOrderExecutionClient<ClientType>::~ToPythonOrderExecutionClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client.reset();
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderRecords(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryOrderRecords(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::QueryExecutionReports(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryExecutionReports(query, queue);
  }

  template<typename ClientType>
  const OrderExecutionPublisher& ToPythonOrderExecutionClient<
      ClientType>::GetOrderSubmissionPublisher() {
    return m_client->GetOrderSubmissionPublisher();
  }

  template<typename ClientType>
  const Order& ToPythonOrderExecutionClient<ClientType>::Submit(
      const OrderFields& fields) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Submit(fields);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Cancel(const Order& order) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Cancel(order);
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Open();
  }

  template<typename ClientType>
  void ToPythonOrderExecutionClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Close();
  }
}
}

#endif
