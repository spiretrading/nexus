#ifndef NEXUS_MARKETDATAVIRTUALMARKETDATACLIENT_HPP
#define NEXUS_MARKETDATAVIRTUALMARKETDATACLIENT_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class VirtualMarketDataClient
      \brief Provides a pure virtual interface to a MarketDataClient.
   */
  class VirtualMarketDataClient : private boost::noncopyable {
    public:
      virtual ~VirtualMarketDataClient();

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue) = 0;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedBookQuote>>& queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedMarketQuote>>& queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedTimeAndSale>>& queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) = 0;

      virtual SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) = 0;

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) = 0;

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualMarketDataClient.
      VirtualMarketDataClient();
  };

  /*! \class WrapperMarketDataClient
      \brief Wraps a MarketDataClient providing it with a virtual interface.
      \tparam ClientType The MarketDataClient to wrap.
   */
  template<typename ClientType>
  class WrapperMarketDataClient : public VirtualMarketDataClient {
    public:

      //! The MarketDataClient to wrap.
      typedef typename Beam::TryDereferenceType<ClientType>::type Client;

      //! Constructs a WrapperMarketDataClient.
      /*!
        \param client The MarketDataClient to wrap.
      */
      template<typename MarketDataClientForward>
      WrapperMarketDataClient(MarketDataClientForward&& client);

      virtual ~WrapperMarketDataClient();

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue);

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      virtual SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security);

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a MarketDataClient into a VirtualMarketDataClient.
  /*!
    \param client The client to wrap.
  */
  template<typename MarketDataClient>
  std::unique_ptr<VirtualMarketDataClient> MakeVirtualMarketDataClient(
      MarketDataClient&& client) {
    return std::make_unique<WrapperMarketDataClient<MarketDataClient>>(
      std::forward<MarketDataClient>(client));
  }

  inline VirtualMarketDataClient::~VirtualMarketDataClient() {}

  inline VirtualMarketDataClient::VirtualMarketDataClient() {}

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  WrapperMarketDataClient<MarketDataClientType>::WrapperMarketDataClient(
      MarketDataClientForward&& client)
      : m_client(std::forward<MarketDataClientForward>(client)) {}

  template<typename MarketDataClientType>
  WrapperMarketDataClient<MarketDataClientType>::~WrapperMarketDataClient() {}

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename MarketDataClientType>
  SecuritySnapshot WrapperMarketDataClient<MarketDataClientType>::
      LoadSecuritySnapshot(const Security& security) {
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename MarketDataClientType>
  SecurityTechnicals WrapperMarketDataClient<MarketDataClientType>::
      LoadSecurityTechnicals(const Security& security) {
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename MarketDataClientType>
  std::vector<SecurityInfo> WrapperMarketDataClient<MarketDataClientType>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::Open() {
    m_client->Open();
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
