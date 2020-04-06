#ifndef NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#define NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include <Beam/IO/OpenState.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a MarketDataClient whose servers are distributed among
   * multiple instances.
   */
  class DistributedMarketDataClient : private boost::noncopyable {
    public:

      /**
       * Constructs a DistributedMarketDataClient.
       * @param countryToMarketDataClients Maps CountryCodes to
       *        MarketDataClients.
       * @param marketToMarketDataClients Maps MarketCodes to MarketDataClients.
       */
      DistributedMarketDataClient(std::unordered_map<
        CountryCode, std::shared_ptr<VirtualMarketDataClient>>
        countryToMarketDataClients, std::unordered_map<
        MarketCode, std::shared_ptr<VirtualMarketDataClient>>
        marketToMarketDataClients);

      ~DistributedMarketDataClient();

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Open();

      void Close();

    private:
      std::unordered_map<CountryCode, std::shared_ptr<VirtualMarketDataClient>>
        m_countryToMarketDataClients;
      std::unordered_map<MarketCode, std::shared_ptr<VirtualMarketDataClient>>
        m_marketToMarketDataClients;
      VirtualMarketDataClient* FindMarketDataClient(MarketCode market);
      VirtualMarketDataClient* FindMarketDataClient(const Security& security);
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline DistributedMarketDataClient::DistributedMarketDataClient(
    std::unordered_map<CountryCode, std::shared_ptr<VirtualMarketDataClient>>
    countryToMarketDataClients, std::unordered_map<MarketCode,
    std::shared_ptr<VirtualMarketDataClient>> marketToMarketDataClients)
    : m_countryToMarketDataClients(std::move(countryToMarketDataClients)),
      m_marketToMarketDataClients(std::move(marketToMarketDataClients)) {}

  inline DistributedMarketDataClient::~DistributedMarketDataClient() {
    Close();
  }

  inline void DistributedMarketDataClient::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryOrderImbalances(query, queue);
  }

  inline void DistributedMarketDataClient::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryOrderImbalances(query, queue);
  }

  inline void DistributedMarketDataClient::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBboQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBboQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBookQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBookQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryMarketQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryMarketQuotes(query, queue);
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryTimeAndSales(query, queue);
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryTimeAndSales(query, queue);
  }

  inline SecuritySnapshot DistributedMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    auto marketDataClient = FindMarketDataClient(security);
    if(marketDataClient == nullptr) {
      return SecuritySnapshot{};
    }
    return marketDataClient->LoadSecuritySnapshot(security);
  }

  inline SecurityTechnicals DistributedMarketDataClient::
      LoadSecurityTechnicals(const Security& security) {
    auto marketDataClient = FindMarketDataClient(security);
    if(marketDataClient == nullptr) {
      return SecurityTechnicals{};
    }
    return marketDataClient->LoadSecurityTechnicals(security);
  }

  inline boost::optional<SecurityInfo>
      DistributedMarketDataClient::LoadSecurityInfo(const Security& security) {
    auto marketDataClient = FindMarketDataClient(security);
    if(marketDataClient == nullptr) {
      return boost::none;
    }
    return marketDataClient->LoadSecurityInfo(security);
  }

  inline std::vector<SecurityInfo> DistributedMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    auto securityInfos = std::vector<SecurityInfo>();
    auto clients =
      std::unordered_set<std::shared_ptr<VirtualMarketDataClient>>();
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

  inline void DistributedMarketDataClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      for(auto& marketDataClient : m_countryToMarketDataClients) {
        marketDataClient.second->Open();
      }
      for(auto& marketDataClient : m_marketToMarketDataClients) {
        marketDataClient.second->Open();
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void DistributedMarketDataClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void DistributedMarketDataClient::Shutdown() {
    m_openState.SetClosed();
  }

  inline VirtualMarketDataClient* DistributedMarketDataClient::
      FindMarketDataClient(MarketCode market) {
    auto marketDataClientIterator = m_marketToMarketDataClients.find(market);
    if(marketDataClientIterator == m_marketToMarketDataClients.end()) {
      return nullptr;
    }
    return marketDataClientIterator->second.get();
  }

  inline VirtualMarketDataClient* DistributedMarketDataClient::
      FindMarketDataClient(const Security& security) {
    auto marketDataClientIterator = m_countryToMarketDataClients.find(
      security.GetCountry());
    if(marketDataClientIterator == m_countryToMarketDataClients.end()) {
      return FindMarketDataClient(security.GetMarket());
    }
    return marketDataClientIterator->second.get();
  }
}

#endif
