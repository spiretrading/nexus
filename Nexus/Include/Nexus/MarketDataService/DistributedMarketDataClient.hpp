#ifndef NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#define NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/join.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a MarketDataClient whose servers are distributed among
   * multiple instances.
   */
  class DistributedMarketDataClient {
    public:

      /**
       * Constructs a DistributedMarketDataClient.
       * @param countryToMarketDataClients Maps CountryCodes to
       *        MarketDataClients.
       * @param marketToMarketDataClients Maps MarketCodes to MarketDataClients.
       */
      DistributedMarketDataClient(
        std::unordered_map<CountryCode, std::shared_ptr<MarketDataClientBox>>
          countryToMarketDataClients,
        std::unordered_map<MarketCode, std::shared_ptr<MarketDataClientBox>>
          marketToMarketDataClients);

      ~DistributedMarketDataClient();

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      std::vector<SecurityInfo> QuerySecurityInfo(
        const SecurityInfoQuery& query);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Close();

    private:
      std::unordered_map<CountryCode, std::shared_ptr<MarketDataClientBox>>
        m_countryToMarketDataClients;
      std::unordered_map<MarketCode, std::shared_ptr<MarketDataClientBox>>
        m_marketToMarketDataClients;
      MarketDataClientBox* FindMarketDataClient(MarketCode market);
      MarketDataClientBox* FindMarketDataClient(const Security& security);
      Beam::IO::OpenState m_openState;

      DistributedMarketDataClient(const DistributedMarketDataClient&) = delete;
      DistributedMarketDataClient& operator =(
        const DistributedMarketDataClient&) = delete;
  };

  inline DistributedMarketDataClient::DistributedMarketDataClient(
    std::unordered_map<CountryCode, std::shared_ptr<MarketDataClientBox>>
      countryToMarketDataClients,
    std::unordered_map<MarketCode, std::shared_ptr<MarketDataClientBox>>
      marketToMarketDataClients)
    : m_countryToMarketDataClients(std::move(countryToMarketDataClients)),
      m_marketToMarketDataClients(std::move(marketToMarketDataClients)) {}

  inline DistributedMarketDataClient::~DistributedMarketDataClient() {
    Close();
  }

  inline void DistributedMarketDataClient::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryOrderImbalances(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryOrderImbalances(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryBboQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryBboQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryBookQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryBookQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryMarketQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryMarketQuotes(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryTimeAndSales(query, std::move(queue));
    }
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    if(auto marketDataClient = FindMarketDataClient(query.GetIndex())) {
      marketDataClient->QueryTimeAndSales(query, std::move(queue));
    }
  }

  inline SecuritySnapshot DistributedMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    if(auto marketDataClient = FindMarketDataClient(security)) {
      return marketDataClient->LoadSecuritySnapshot(security);
    }
    return SecuritySnapshot();
  }

  inline std::vector<SecurityInfo> DistributedMarketDataClient::
      QuerySecurityInfo(const SecurityInfoQuery& query) {
    auto client = [&] {
      if(!m_countryToMarketDataClients.empty()) {
        return m_countryToMarketDataClients.begin()->second;
      }
      if(m_marketToMarketDataClients.empty()) {
        return std::shared_ptr<MarketDataClientBox>();
      }
      return m_marketToMarketDataClients.begin()->second;
    }();
    if(client) {
      return client->QuerySecurityInfo(query);
    }
    return {};
  }

  inline std::vector<SecurityInfo> DistributedMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    auto securityInfos = std::vector<SecurityInfo>();
    auto clients = std::unordered_set<std::shared_ptr<MarketDataClientBox>>();
    for(auto& client :
        m_countryToMarketDataClients | boost::adaptors::map_values) {
      clients.insert(client);
    }
    for(auto& client : clients) {
      auto result = client->LoadSecurityInfoFromPrefix(prefix);
      securityInfos.insert(securityInfos.end(), result.begin(), result.end());
    }
    return securityInfos;
  }

  inline void DistributedMarketDataClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto clientCount =
      std::unordered_map<std::shared_ptr<MarketDataClientBox>, int>();
    for(auto& client : boost::range::join(
        m_countryToMarketDataClients | boost::adaptors::map_values,
        m_marketToMarketDataClients | boost::adaptors::map_values)) {
      ++clientCount[client];
    }
    auto closeGroup = Beam::Routines::RoutineHandlerGroup();
    for(auto& client : clientCount) {
      if(client.first.use_count() == client.second + 1) {
        closeGroup.Spawn([client = client.first] {
          client->Close();
        });
      }
    }
    closeGroup.Wait();
    m_openState.Close();
  }

  inline MarketDataClientBox* DistributedMarketDataClient::FindMarketDataClient(
      MarketCode market) {
    auto marketDataClientIterator = m_marketToMarketDataClients.find(market);
    if(marketDataClientIterator == m_marketToMarketDataClients.end()) {
      return nullptr;
    }
    return marketDataClientIterator->second.get();
  }

  inline MarketDataClientBox* DistributedMarketDataClient::FindMarketDataClient(
      const Security& security) {
    auto marketDataClientIterator = m_countryToMarketDataClients.find(
      security.GetCountry());
    if(marketDataClientIterator == m_countryToMarketDataClients.end()) {
      return FindMarketDataClient(security.GetMarket());
    }
    return marketDataClientIterator->second.get();
  }
}

#endif
