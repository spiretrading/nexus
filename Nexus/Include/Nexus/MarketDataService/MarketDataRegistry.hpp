#ifndef NEXUS_MARKET_DATA_REGISTRY_HPP
#define NEXUS_MARKET_DATA_REGISTRY_HPP
#include <memory>
#include <unordered_set>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Collections/Trie.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketEntry.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

namespace Nexus::MarketDataService {
namespace Details {
  template<typename DataStore>
  Money LoadClosePrice(const Security& security, DataStore& dataStore) {
    auto marketCenters =
      [&] () -> std::vector<std::string> {
        if(security.GetCountry() == DefaultCountries::CA()) {
          return {"TSE", "CDX", "CNQ"};
        } else if(security.GetCountry() == DefaultCountries::AU()) {
          return {"ASX"};
        } else if(!security.GetMarket().IsEmpty()) {
          return {std::string{security.GetMarket().GetData()}};
        } else {
          return {};
        }
      }();
    for(auto& marketCenter : marketCenters) {
      auto queryMarketCode = Beam::Queries::StringValue(marketCenter);
      auto marketCodeExpression = Beam::Queries::ConstantExpression(
        queryMarketCode);
      auto parameterExpression = Beam::Queries::ParameterExpression(
        0, Nexus::Queries::TimeAndSaleType());
      auto accessExpression = Beam::Queries::MemberAccessExpression(
        "market_center", Beam::Queries::StringType(), parameterExpression);
      auto equalExpression = Beam::Queries::MakeEqualsExpression(
        marketCodeExpression, accessExpression);
      auto previousCloseQuery = MarketDataService::SecurityMarketDataQuery();
      previousCloseQuery.SetIndex(security);
      previousCloseQuery.SetRange(Beam::Queries::Range::Total());
      previousCloseQuery.SetSnapshotLimit(
        Beam::Queries::SnapshotLimit::Type::TAIL, 1);
      previousCloseQuery.SetFilter(equalExpression);
      auto result = dataStore.LoadTimeAndSales(previousCloseQuery);
      if(!result.empty()) {
        return result.back()->m_price;
      }
    }
    return Money::ZERO;
  }
}

  /** Keeps and updates the registry of market data. */
  class MarketDataRegistry {
    public:

      /** Constructs a MarketDataRegistry. */
      MarketDataRegistry();

      /**
       * Adds or updates a SecurityInfo to this registry.
       * @param securityInfo The SecurityInfo to add or update.
       */
      void Add(const SecurityInfo& securityInfo);

      /**
       * Returns a list of SecurityInfo's matching a prefix.
       * @param prefix The prefix to search for.
       * @return The list of SecurityInfo's that match the <i>prefix</i>.
       */
      std::vector<SecurityInfo> SearchSecurityInfo(const std::string& prefix);

      /**
       * Returns a Security's primary listing.
       * @param security The Security to search.
       * @return The <i>security</i>'s primary listing, if no such listing
       *         exists then the MarketCode will be set to empty.
       */
      Security GetPrimaryListing(const Security& security);

      /**
       * Publishes an OrderImbalance.
       * @param orderImbalance The OrderImbalance to publish.
       * @param sourceId The id of the source setting the value.
       * @param dataStore Used to initialize the market's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance,
        int sourceId, DataStore& dataStore, const F& f);

      /**
       * Publishes a BboQuote.
       * @param bboQuote The BboQuote to publish.
       * @param sourceId The id of the source setting the value.
       * @param dataStore Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void PublishBboQuote(const SecurityBboQuote& bboQuote, int sourceId,
        DataStore& dataStore, const F& f);

      /**
       * Sets a MarketQuote.
       * @param marketQuote The MarketQuote to set.
       * @param sourceId The id of the source setting the value.
       * @param dataStore Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void PublishMarketQuote(const SecurityMarketQuote& marketQuote,
        int sourceId, DataStore& dataStore, const F& f);

      /**
       * Updates a BookQuote.
       * @param delta The BookQuote storing the change.
       * @param sourceId The id of the source setting the value.
       * @param dataStore Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void UpdateBookQuote(const SecurityBookQuote& delta, int sourceId,
        DataStore& dataStore, const F& f);

      /**
       * Publishes a TimeAndSale.
       * @param timeAndSale The TimeAndSale to publish.
       * @param sourceId The id of the source setting the value.
       * @param dataStore Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale,
        int sourceId, DataStore& dataStore, const F& f);

      /**
       * Returns a Security's SecurityTechnicals.
       * @param security The Security whose SecurityTechnicals is to be
       *        returned.
       * @return A snapshot of the <i>security</i>'s SecurityTechnicals.
       */
      boost::optional<SecurityTechnicals> FindSecurityTechnicals(
        const Security& security);

      /**
       * Returns a Security's real time snapshot.
       * @param security The Security whose snapshot is to be returned.
       * @return The real-time snapshot of the <i>security</i>.
       */
      boost::optional<SecuritySnapshot> FindSnapshot(const Security& security);

      /**
       * Clears market data that originated from a specified source.
       * @param sourceId The id of the source to clear.
       */
      void Clear(int sourceId);

    private:
      using SyncMarketEntry = Beam::Threading::Sync<MarketEntry>;
      using SyncSecurityEntry = Beam::Threading::Sync<SecurityEntry>;
      Beam::Threading::Sync<rtv::Trie<char, SecurityInfo>> m_securityDatabase;
      Beam::SynchronizedUnorderedSet<Security> m_verifiedSecurities;
      Beam::SynchronizedUnorderedMap<MarketCode,
        std::shared_ptr<Beam::Remote<SyncMarketEntry>>> m_marketEntries;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<Beam::Remote<SyncSecurityEntry>>> m_securityEntries;

      MarketDataRegistry(const MarketDataRegistry&) = delete;
      MarketDataRegistry& operator =(const MarketDataRegistry&) = delete;
      template<typename DataStore>
      boost::optional<SyncMarketEntry&> LoadMarketEntry(MarketCode market,
        DataStore& dataStore);
      template<typename DataStore>
      boost::optional<SyncSecurityEntry&> LoadSecurityEntry(
        const Security& security, DataStore& dataStore);
  };

  inline MarketDataRegistry::MarketDataRegistry()
    : m_securityDatabase('\0') {}

  inline void MarketDataRegistry::Add(const SecurityInfo& securityInfo) {
    auto key = ToString(securityInfo.m_security);
    auto name = boost::to_upper_copy(securityInfo.m_name);
    Beam::Threading::With(m_securityDatabase, [&] (auto& securityDatabase) {
      securityDatabase[key.c_str()] = securityInfo;
      securityDatabase[name.c_str()] = securityInfo;
    });
    m_verifiedSecurities.Update(securityInfo.m_security);
  }

  inline std::vector<SecurityInfo> MarketDataRegistry::SearchSecurityInfo(
      const std::string& prefix) {
    static const auto MAX_MATCH_COUNT = 8;
    auto matches = std::unordered_set<SecurityInfo>();
    auto uppercasePrefix = boost::to_upper_copy(prefix);
    Beam::Threading::With(m_securityDatabase, [&] (auto& securityDatabase) {
      for(auto i = securityDatabase.startsWith(uppercasePrefix.c_str());
          i != securityDatabase.end(); ++i) {
        matches.insert(*i->second);
      }
    });
    auto i = matches.begin();
    while(i != matches.end()) {
      auto entry = m_securityEntries.FindValue(i->m_security);
      if(!entry || !(*entry)->IsAvailable()) {
        i = matches.erase(i);
      } else {
        Beam::Threading::With(***entry, [&] (auto& entry) {
          if((*entry.GetBboQuote())->m_ask.m_price == Money::ZERO) {
            i = matches.erase(i);
          } else {
            ++i;
          }
        });
      }
    }
    auto result = std::vector<SecurityInfo>(matches.begin(), matches.end());
    if(result.size() > MAX_MATCH_COUNT) {
      result.erase(result.begin() + MAX_MATCH_COUNT, result.end());
    }
    return result;
  }

  inline Security MarketDataRegistry::GetPrimaryListing(
      const Security& security) {
    if(security.GetSymbol().empty() ||
        security.GetCountry() == CountryCode::NONE) {
      return Security(security.GetSymbol(), CountryCode::NONE);
    }
    if(auto verifiedSecurity = m_verifiedSecurities.FindValue(security)) {
      return std::move(*verifiedSecurity);
    }
    auto entry = m_securityEntries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return Security(security.GetSymbol(), security.GetCountry());
    }
    return Beam::Threading::With(***entry, [&] (auto& entry) {
      if(entry.GetSecurity().GetMarket().IsEmpty()) {
        return Security(security.GetSymbol(), security.GetCountry());
      }
      return entry.GetSecurity();
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance, int sourceId,
      DataStore& dataStore, const F& f) {
    auto securityEntry = LoadSecurityEntry(orderImbalance->m_security,
      dataStore);
    if(!securityEntry) {
      return;
    }
    auto marketEntry = LoadMarketEntry(orderImbalance.GetIndex(), dataStore);
    if(!marketEntry) {
      return;
    }
    auto [security, referencePrice] = Beam::Threading::With(*securityEntry,
      [&] (auto& securityEntry) {
        auto referencePrice = [&] {
          if(orderImbalance->m_referencePrice == Money::ZERO) {
            auto& bbo = **securityEntry.GetBboQuote();
            return Pick(orderImbalance->m_side, bbo.m_ask.m_price,
              bbo.m_bid.m_price);
          }
          return orderImbalance->m_referencePrice;
        }();
        return std::tuple(securityEntry.GetSecurity(), referencePrice);
      });
    Beam::Threading::With(*marketEntry, [&] (auto& entry) {
      auto sanitizedOrderImbalance = OrderImbalance(orderImbalance);
      sanitizedOrderImbalance.m_security = std::move(security);
      sanitizedOrderImbalance.m_referencePrice = std::move(referencePrice);
      if(auto sequencedOrderImbalance = entry.PublishOrderImbalance(
          std::move(sanitizedOrderImbalance), sourceId)) {
        f(*sequencedOrderImbalance);
      }
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::PublishBboQuote(const SecurityBboQuote& bboQuote,
      int sourceId, DataStore& dataStore, const F& f) {
    auto entry = LoadSecurityEntry(bboQuote.GetIndex(), dataStore);
    if(!entry) {
      return;
    }
    Beam::Threading::With(*entry, [&] (auto& entry) {
      if(entry.GetSecurity().GetMarket().IsEmpty()) {
        if(auto verifiedSecurity =
            m_verifiedSecurities.FindValue(bboQuote.GetIndex())) {
          entry.SetSecurity(std::move(*verifiedSecurity));
        } else {
          entry.SetSecurity(bboQuote.GetIndex());
        }
        auto key = ToString(entry.GetSecurity());
        auto info = SecurityInfo(entry.GetSecurity(), key, "", 0);
        Beam::Threading::With(m_securityDatabase, [&] (auto& securityDatabase) {
          securityDatabase.insert(key.c_str(), info);
        });
      }
      if(auto sequencedBboQuote =
          entry.PublishBboQuote(std::move(bboQuote), sourceId)) {
        f(*sequencedBboQuote);
      }
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote, int sourceId,
      DataStore& dataStore, const F& f) {
    auto entry = LoadSecurityEntry(marketQuote.GetIndex(), dataStore);
    if(!entry) {
      return;
    }
    Beam::Threading::With(*entry, [&] (auto& entry) {
      if(auto sequencedMarketQuote =
          entry.PublishMarketQuote(std::move(marketQuote), sourceId)) {
        f(*sequencedMarketQuote);
      }
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::UpdateBookQuote(const SecurityBookQuote& delta,
      int sourceId, DataStore& dataStore, const F& f) {
    auto entry = LoadSecurityEntry(delta.GetIndex(), dataStore);
    if(!entry) {
      return;
    }
    Beam::Threading::With(*entry, [&] (auto& entry) {
      if(auto sequencedBookQuote =
          entry.UpdateBookQuote(std::move(delta), sourceId)) {
        f(*sequencedBookQuote);
      }
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale, int sourceId,
      DataStore& dataStore, const F& f) {
    auto entry = LoadSecurityEntry(timeAndSale.GetIndex(), dataStore);
    if(!entry) {
      return;
    }
    Beam::Threading::With(*entry, [&] (auto& entry) {
      if(auto sequencedTimeAndSale =
          entry.PublishTimeAndSale(std::move(timeAndSale), sourceId)) {
        f(*sequencedTimeAndSale);
      }
    });
  }

  inline boost::optional<SecurityTechnicals>
      MarketDataRegistry::FindSecurityTechnicals(const Security& security) {
    auto entry = m_securityEntries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return boost::none;
    }
    return Beam::Threading::With(***entry, [&] (auto& entry) {
      return entry.GetSecurityTechnicals();
    });
  }

  inline boost::optional<SecuritySnapshot> MarketDataRegistry::FindSnapshot(
      const Security& security) {
    auto entry = m_securityEntries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return boost::none;
    }
    return Beam::Threading::With(***entry, [&] (auto& entry) {
      return entry.LoadSnapshot();
    });
  }

  inline void MarketDataRegistry::Clear(int sourceId) {
    auto entries =
      std::vector<std::shared_ptr<Beam::Remote<SyncSecurityEntry>>>();
    m_securityEntries.With([&] (auto& securityEntries) {
      for(auto& entry : securityEntries | boost::adaptors::map_values) {
        entries.push_back(entry);
      }
    });
    for(auto& entry : entries) {
      if(entry->IsAvailable()) {
        Beam::Threading::With(**entry, [&] (auto& entry) {
          entry.Clear(sourceId);
        });
      }
    }
  }

  template<typename DataStore>
  inline boost::optional<MarketDataRegistry::SyncMarketEntry&>
      MarketDataRegistry::LoadMarketEntry(MarketCode market,
        DataStore& dataStore) {
    if(market == MarketCode()) {
      return boost::none;
    }
    auto entry = m_marketEntries.GetOrInsert(market, [&] {
      return std::make_shared<Beam::Remote<SyncMarketEntry>>(
        [&] (auto& entry) {
          entry.emplace(market, LoadInitialSequences(dataStore, market));
        });
    });
    return **entry;
  }

  template<typename DataStore>
  boost::optional<MarketDataRegistry::SyncSecurityEntry&>
      MarketDataRegistry::LoadSecurityEntry(const Security& security,
      DataStore& dataStore) {
    if(security.GetSymbol().empty() ||
        security.GetCountry() == CountryCode::NONE) {
      return boost::none;
    }
    auto entry = m_securityEntries.GetOrInsert(security, [&] {
      return std::make_shared<Beam::Remote<SyncSecurityEntry>>(
        [&] (auto& entry) {
          auto sanitizedSecurity = GetPrimaryListing(security);
          auto initialSequences =
            LoadInitialSequences(dataStore, sanitizedSecurity);
          auto closePrice =
            Details::LoadClosePrice(sanitizedSecurity, dataStore);
          entry.emplace(sanitizedSecurity, closePrice, initialSequences);
        });
    });
    return **entry;
  }
}

#endif
