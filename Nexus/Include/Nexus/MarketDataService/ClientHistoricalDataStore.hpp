#ifndef NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataClient for use as a HistoricalDataStore.
   * @param <C> The type of MarketDataClient to wrap.
   */
  template<typename C>
  class ClientHistoricalDataStore {
    public:

      /** The type of MarketDataClient to wrap. */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a ClientHistoricalDataStore.
       * @param client Initializes the client to wrap.
       */
      template<typename CF>
      explicit ClientHistoricalDataStore(CF&& client);
      ~ClientHistoricalDataStore();
      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);
      void store(const SecurityInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBboQuote& quote);
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBookQuote& quote);
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityTimeAndSale& time_and_sale);
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);
      void close();

    private:
      using ClientType = std::remove_reference_t<
        decltype(*std::declval<Beam::GetOptionalLocalPtr<C>>())>;
      Beam::GetOptionalLocalPtr<C> m_client;
      Beam::IO::OpenState m_open_state;

      ClientHistoricalDataStore(const ClientHistoricalDataStore&) = delete;
      ClientHistoricalDataStore& operator =(
        const ClientHistoricalDataStore&) = delete;
      template<typename T, typename Query>
      std::vector<T> submit(const Query& query);
  };

  template<typename C>
  template<typename CF>
  ClientHistoricalDataStore<C>::ClientHistoricalDataStore(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  ClientHistoricalDataStore<C>::~ClientHistoricalDataStore() {
    close();
  }

  template<typename C>
  std::vector<SecurityInfo> ClientHistoricalDataStore<C>::load_security_info(
      const SecurityInfoQuery& query) {
    return m_client->query(query);
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::store(const SecurityInfo& info) {}

  template<typename C>
  std::vector<SequencedOrderImbalance>
      ClientHistoricalDataStore<C>::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return submit<SequencedOrderImbalance>(query);
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const SequencedVenueOrderImbalance& imbalance) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedVenueOrderImbalance>& imbalances) {}

  template<typename C>
  std::vector<SequencedBboQuote>
      ClientHistoricalDataStore<C>::load_bbo_quotes(
        const SecurityMarketDataQuery& query) {
    return submit<SequencedBboQuote>(query);
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const SequencedSecurityBboQuote& quote) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedSecurityBboQuote>& quotes) {}

  template<typename C>
  std::vector<SequencedBookQuote>
      ClientHistoricalDataStore<C>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return submit<SequencedBookQuote>(query);
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const SequencedSecurityBookQuote& quote) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedSecurityBookQuote>& quotes) {}

  template<typename C>
  std::vector<SequencedTimeAndSale>
      ClientHistoricalDataStore<C>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return submit<SequencedTimeAndSale>(query);
  }

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const SequencedSecurityTimeAndSale& time_and_sale) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {}

  template<typename C>
  void ClientHistoricalDataStore<C>::close() {
    m_open_state.Close();
  }

  template<typename C>
  template<typename T, typename Query>
  std::vector<T> ClientHistoricalDataStore<C>::submit(const Query& query) {
    auto queue = std::make_shared<Beam::Queue<T>>();
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      auto revised_query = query;
      revised_query.SetRange(
        query.GetRange().GetStart(), Beam::Queries::Sequence::Present());
      m_client->query(revised_query, queue);
    } else {
      m_client->query(query, queue);
    }
    auto matches = std::vector<T>();
    Beam::Flush(queue, std::back_inserter(matches));
    return matches;
  }
}

#endif
