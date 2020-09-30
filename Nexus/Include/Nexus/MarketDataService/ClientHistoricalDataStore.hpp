#ifndef NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#include <type_traits>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/Queue.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataClient for use as a HistoricalDataStore.
   * @param <C> The type of MarketDataClient to wrap.
   */
  template<typename C>
  class ClientHistoricalDataStore : private boost::noncopyable {
    public:

      /** The type of MarketDataClient to wrap. */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a ClientHistoricalDataStore.
       * @param client Initializes the client to wrap.
       */
      template<typename CF>
      ClientHistoricalDataStore(CF&& client);

      ~ClientHistoricalDataStore();

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadAllSecurityInfo();

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

      void Store(const SecurityInfo& info);

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

      void Close();

    private:
      using ClientType = std::remove_reference_t<
        decltype(*std::declval<Beam::GetOptionalLocalPtr<C>>())>;
      Beam::GetOptionalLocalPtr<C> m_client;
      Beam::IO::OpenState m_openState;

      template<typename T, typename Query, typename F>
      std::vector<T> SubmitQuery(const Query& query, F f);
  };

  template<typename C>
  template<typename CF>
  ClientHistoricalDataStore<C>::ClientHistoricalDataStore(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  ClientHistoricalDataStore<C>::~ClientHistoricalDataStore() {
    Close();
  }

  template<typename C>
  boost::optional<SecurityInfo> ClientHistoricalDataStore<C>::LoadSecurityInfo(
      const Security& security) {
    return m_client->LoadSecurityInfo(security);
  }

  template<typename C>
  std::vector<SecurityInfo> ClientHistoricalDataStore<C>::
      LoadAllSecurityInfo() {
    return {};
  }

  template<typename C>
  std::vector<SequencedOrderImbalance> ClientHistoricalDataStore<C>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    using MemberType = void (ClientType::*)(const MarketWideDataQuery&,
      Beam::ScopedQueueWriter<SequencedOrderImbalance>);
    return SubmitQuery<SequencedOrderImbalance>(query,
      static_cast<MemberType>(&ClientType::QueryOrderImbalances));
  }

  template<typename C>
  std::vector<SequencedBboQuote> ClientHistoricalDataStore<C>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      Beam::ScopedQueueWriter<SequencedBboQuote>);
    return SubmitQuery<SequencedBboQuote>(query,
      static_cast<MemberType>(&ClientType::QueryBboQuotes));
  }

  template<typename C>
  std::vector<SequencedBookQuote> ClientHistoricalDataStore<C>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      Beam::ScopedQueueWriter<SequencedBookQuote>);
    return SubmitQuery<SequencedBookQuote>(query,
      static_cast<MemberType>(&ClientType::QueryBookQuotes));
  }

  template<typename C>
  std::vector<SequencedMarketQuote> ClientHistoricalDataStore<C>::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      Beam::ScopedQueueWriter<SequencedMarketQuote>);
    return SubmitQuery<SequencedMarketQuote>(query,
      static_cast<MemberType>(&ClientType::QueryMarketQuotes));
  }

  template<typename C>
  std::vector<SequencedTimeAndSale> ClientHistoricalDataStore<C>::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    using MemberType = void (ClientType::*)(const SecurityMarketDataQuery&,
      Beam::ScopedQueueWriter<SequencedTimeAndSale>);
    return SubmitQuery<SequencedTimeAndSale>(query,
      static_cast<MemberType>(&ClientType::QueryTimeAndSales));
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(const SecurityInfo& info) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const SequencedMarketOrderImbalance& orderImbalance) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const SequencedSecurityBboQuote& bboQuote) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const std::vector<SequencedSecurityBboQuote>& bboQuotes) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const SequencedSecurityMarketQuote& marketQuote) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const SequencedSecurityBookQuote& bookQuote) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const std::vector<SequencedSecurityBookQuote>& bookQuotes) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const SequencedSecurityTimeAndSale& timeAndSale) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Store(
    const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::Close() {
    m_openState.Close();
  }

  template<typename C>
  template<typename T, typename Query, typename F>
  std::vector<T> ClientHistoricalDataStore<C>::SubmitQuery(const Query& query,
      F f) {
    auto queue = std::make_shared<Beam::Queue<T>>();
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      auto revisedQuery = query;
      revisedQuery.SetRange(query.GetRange().GetStart(),
        Beam::Queries::Sequence::Present());
      ((*m_client).*f)(revisedQuery, queue);
    } else {
      ((*m_client).*f)(query, queue);
    }
    auto matches = std::vector<T>();
    Beam::Flush(queue, std::back_inserter(matches));
    return matches;
  }
}

#endif
