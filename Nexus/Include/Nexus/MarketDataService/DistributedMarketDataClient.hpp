#ifndef NEXUS_DISTRIBUTEDMARKETDATACLIENT_HPP
#define NEXUS_DISTRIBUTEDMARKETDATACLIENT_HPP
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <Beam/IO/OpenState.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class DistributedMarketDataClient
      \brief Implements a MarketDataClient whose servers are distributed among
             multiple instances.
   */
  class DistributedMarketDataClient : private boost::noncopyable {
    public:

      //! Constructs a DistributedMarketDataClient.
      /*!
        \param countryToMarketDataClients Maps CountryCodes to
               MarketDataClients.
        \param marketToMarketDataClients Maps MarketCodes to MarketDataClients.
      */
      DistributedMarketDataClient(std::unordered_map<
        CountryCode, std::shared_ptr<VirtualMarketDataClient>>
        countryToMarketDataClients, std::unordered_map<
        MarketCode, std::shared_ptr<VirtualMarketDataClient>>
        marketToMarketDataClients);

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
      : m_countryToMarketDataClients{std::move(countryToMarketDataClients)},
        m_marketToMarketDataClients{std::move(marketToMarketDataClients)} {}

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
    std::cout << "BBO: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBboQuotes(query, queue);
    std::cout << "Done BBO: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    std::cout << "BBO: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBboQuotes(query, queue);
    std::cout << "Done BBO: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    std::cout << "Book: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBookQuotes(query, queue);
    std::cout << "Done Book: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    std::cout << "Book: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryBookQuotes(query, queue);
    std::cout << "Done Book: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    std::cout << "Market: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryMarketQuotes(query, queue);
    std::cout << "Done market: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    std::cout << "Market: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryMarketQuotes(query, queue);
    std::cout << "Done market: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    std::cout << "TAS: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryTimeAndSales(query, queue);
    std::cout << "Done tas: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline void DistributedMarketDataClient::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    std::cout << "TAS: " << query.GetIndex().GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(query.GetIndex());
    if(marketDataClient == nullptr) {
      queue->Break();
      return;
    }
    marketDataClient->QueryTimeAndSales(query, queue);
    std::cout << "Done tas: " << query.GetIndex().GetSymbol() << std::endl;
  }

  inline SecuritySnapshot DistributedMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    std::cout << "Snapshot: " << security.GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(security);
    if(marketDataClient == nullptr) {
      return SecuritySnapshot{};
    }
    std::cout << "Done." << std::endl;
    return marketDataClient->LoadSecuritySnapshot(security);
  }

  inline SecurityTechnicals DistributedMarketDataClient::
      LoadSecurityTechnicals(const Security& security) {
    std::cout << "Tech: " << security.GetSymbol() << std::endl;
    auto marketDataClient = FindMarketDataClient(security);
    if(marketDataClient == nullptr) {
      return SecurityTechnicals{};
    }
    std::cout << "Done tech: " << security.GetSymbol() << std::endl;
    return marketDataClient->LoadSecurityTechnicals(security);
  }

  inline std::vector<SecurityInfo> DistributedMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return {};
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
}

#endif
