#ifndef NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#include <type_traits>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/DefaultValue.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Wraps a MarketDataClient for use as a HistoricalDataStore.
      \tparam MarketDataClientType The type of MarketDataClient to wrap.
   */
  template<typename MarketDataClientType>
  class ClientHistoricalDataStore : private boost::noncopyable {
    public:

      //! The type of MarketDataClient to wrap.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! Constructs a ClientHistoricalDataStore.
      /*!
        \param client Initializes the client to wrap.
      */
      template<typename MarketDataClientForward>
      ClientHistoricalDataStore(MarketDataClientForward&& client);

      ~ClientHistoricalDataStore();

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_client;
      Beam::IO::OpenState m_openState;

      template<typename T, typename Query, typename F>
      std::vector<T> SubmitQuery(const Query& query, F f);
      void Shutdown();
  };

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  ClientHistoricalDataStore<MarketDataClientType>::ClientHistoricalDataStore(
      MarketDataClientForward&& client)
      : m_client{std::forward<MarketDataClientForward>(client)} {}

  template<typename MarketDataClientType>
  ClientHistoricalDataStore<MarketDataClientType>::
      ~ClientHistoricalDataStore() {
    Close();
  }

  template<typename MarketDataClientType>
  std::vector<SequencedOrderImbalance> ClientHistoricalDataStore<
      MarketDataClientType>::LoadOrderImbalances(
      const MarketWideDataQuery& query) {
    using ClientType =
      typename std::remove_reference<decltype(*m_client)>::type;
    using MemberType = void (ClientType::*)(const MarketWideDataQuery&,
      const std::shared_ptr<Beam::QueueWriter<SequencedOrderImbalance>>&);
    return SubmitQuery<SequencedOrderImbalance>(query,
      static_cast<MemberType>(&ClientType::QueryOrderImbalances));
  }

  template<typename MarketDataClientType>
  std::vector<SequencedBboQuote> ClientHistoricalDataStore<
      MarketDataClientType>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    using ClientType =
      typename std::remove_reference<decltype(*m_client)>::type;
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>&);
    return SubmitQuery<SequencedBboQuote>(query,
      static_cast<MemberType>(&ClientType::QueryBboQuotes));
  }

  template<typename MarketDataClientType>
  std::vector<SequencedBookQuote> ClientHistoricalDataStore<
      MarketDataClientType>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    using ClientType =
      typename std::remove_reference<decltype(*m_client)>::type;
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>&);
    return SubmitQuery<SequencedBookQuote>(query,
      static_cast<MemberType>(&ClientType::QueryBookQuotes));
  }

  template<typename MarketDataClientType>
  std::vector<SequencedMarketQuote> ClientHistoricalDataStore<
      MarketDataClientType>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    using ClientType =
      typename std::remove_reference<decltype(*m_client)>::type;
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>&);
    return SubmitQuery<SequencedMarketQuote>(query,
      static_cast<MemberType>(&ClientType::QueryMarketQuotes));
  }

  template<typename MarketDataClientType>
  std::vector<SequencedTimeAndSale> ClientHistoricalDataStore<
      MarketDataClientType>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    using ClientType =
      typename std::remove_reference<decltype(*m_client)>::type;
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>&);
    return SubmitQuery<SequencedTimeAndSale>(query,
      static_cast<MemberType>(&ClientType::QueryTimeAndSales));
  }

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const SequencedSecurityBboQuote& bboQuote) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const SequencedSecurityBookQuote& bookQuote) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {}

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataClientType>
  template<typename T, typename Query, typename F>
  std::vector<T> ClientHistoricalDataStore<MarketDataClientType>::SubmitQuery(
      const Query& query, F f) {
    auto queue = std::make_shared<Beam::Queue<T>>();
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      auto revisedQuery = query;
      revisedQuery.SetRange(query.GetRange().GetStart(),
        Beam::Queries::Sequence::Present());
      ((*m_client).*f)(revisedQuery, queue);
    } else {
      ((*m_client).*f)(query, queue);
    }
    std::vector<T> matches;
    Beam::FlushQueue(queue, std::back_inserter(matches));
    return matches;
  }

  template<typename MarketDataClientType>
  void ClientHistoricalDataStore<MarketDataClientType>::Shutdown() {
    m_openState.SetClosed();
  }
}

#endif
