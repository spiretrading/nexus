#ifndef NEXUS_VIRTUALORDEREXECUTIONCLIENT_HPP
#define NEXUS_VIRTUALORDEREXECUTIONCLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class VirtualOrderExecutionClient
      \brief Provides a pure virtual interface to an OrderExecutionClient.
   */
  class VirtualOrderExecutionClient : private boost::noncopyable {
    public:
      virtual ~VirtualOrderExecutionClient() = default;

      virtual void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) = 0;

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue) = 0;

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) = 0;

      virtual void QueryExecutionReports(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue) = 0;

      virtual const OrderExecutionPublisher& GetOrderSubmissionPublisher() = 0;

      virtual const Order& Submit(const OrderFields& fields) = 0;

      virtual void Cancel(const Order& order) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualOrderExecutionClient.
      VirtualOrderExecutionClient() = default;
  };

  /*! \class WrapperOrderExecutionClient
      \brief Wraps a OrderExecutionClient providing it with a virtual interface.
      \tparam ClientType The type of OrderExecutionClient to wrap.
   */
  template<typename ClientType>
  class WrapperOrderExecutionClient : public VirtualOrderExecutionClient {
    public:

      //! The OrderExecutionClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperOrderExecutionClient.
      /*!
        \param client The OrderExecutionClient to wrap.
      */
      template<typename OrderExecutionClientForward>
      WrapperOrderExecutionClient(OrderExecutionClientForward&& client);

      virtual ~WrapperOrderExecutionClient();

      virtual void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue);

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue);

      virtual void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue);

      virtual void QueryExecutionReports(const AccountQuery& query,
        const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue);

      virtual const OrderExecutionPublisher& GetOrderSubmissionPublisher();

      virtual const Order& Submit(const OrderFields& fields);

      virtual void Cancel(const Order& order);

      virtual void Open();

      virtual void Close();

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps an OrderExecutionClient into a VirtualOrderExecutionClient.
  /*!
    \param client The client to wrap.
  */
  template<typename OrderExecutionClient>
  std::unique_ptr<VirtualOrderExecutionClient> MakeVirtualOrderExecutionClient(
      OrderExecutionClient&& client) {
    return std::make_unique<WrapperOrderExecutionClient<OrderExecutionClient>>(
      std::forward<OrderExecutionClient>(client));
  }

  template<typename ClientType>
  template<typename OrderExecutionClientForward>
  WrapperOrderExecutionClient<ClientType>::WrapperOrderExecutionClient(
      OrderExecutionClientForward&& client)
      : m_client(std::forward<OrderExecutionClientForward>(client)) {}

  template<typename ClientType>
  WrapperOrderExecutionClient<ClientType>::~WrapperOrderExecutionClient() {}

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderRecords(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderRecord>>& queue) {
    m_client->QueryOrderRecords(query, queue);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedOrder>>& queue) {
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryOrderSubmissions(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<const Order*>>& queue) {
    m_client->QueryOrderSubmissions(query, queue);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::QueryExecutionReports(
      const AccountQuery& query,
      const std::shared_ptr<Beam::QueueWriter<ExecutionReport>>& queue) {
    m_client->QueryExecutionReports(query, queue);
  }

  template<typename ClientType>
  const OrderExecutionPublisher& WrapperOrderExecutionClient<ClientType>::
      GetOrderSubmissionPublisher() {
    return m_client->GetOrderSubmissionPublisher();
  }

  template<typename ClientType>
  const Order& WrapperOrderExecutionClient<ClientType>::Submit(
      const OrderFields& fields) {
    return m_client->Submit(fields);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Cancel(const Order& order) {
    m_client->Cancel(order);
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperOrderExecutionClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
