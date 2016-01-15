#ifndef NEXUS_MARKETDATAREGISTRY_HPP
#define NEXUS_MARKETDATAREGISTRY_HPP
#include <array>
#include <memory>
#include <unordered_set>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <Beam/Utilities/Trie.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketEntry.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataRegistry
      \brief Keeps and updates the registry of market data.
   */
  class MarketDataRegistry : private boost::noncopyable {
    public:

      //! Constructs a MarketDataRegistry.
      MarketDataRegistry();

      //! Adds or updates a SecurityInfo to this registry.
      /*!
        \param securityInfo The SecurityInfo to add or update.
      */
      void Add(const SecurityInfo& securityInfo);

      //! Returns a list of SecurityInfo's matching a prefix.
      /*!
        \param prefix The prefix to search for.
        \return The list of SecurityInfo's that match the <i>prefix</i>.
      */
      std::vector<SecurityInfo> SearchSecurityInfo(
        const std::string& prefix);

      //! Returns a Security's primary listing.
      /*!
        \param security The Security to search.
        \return The <i>security</i>'s primary listing, if no such listing exists
                then the MarketCode will be set to empty.
      */
      Security GetPrimaryListing(const Security& security);

      //! Publishes an OrderImbalance.
      /*!
        \param orderImbalance The OrderImbalance to publish.
        \param sourceId The id of the source setting the value.
        \param securityInitialSequenceLoader Loads initial Sequences for the
               Security.
        \param marketInitialSequenceLoader Loads initial Sequences for the
               Market.
        \param f Receives synchronized access to the updated data.
      */
      template<typename SecurityInitialSequenceLoader,
        typename MarketInitialSequenceLoader, typename F>
      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance,
        int sourceId,
        const SecurityInitialSequenceLoader& securityInitialSequenceLoader,
        const MarketInitialSequenceLoader& marketInitialSequenceLoader,
        const F& f);

      //! Publishes a BboQuote.
      /*!
        \param bboQuote The BboQuote to publish.
        \param sourceId The id of the source setting the value.
        \param initialSequenceLoader Loads initial Sequences for the Security.
        \param f Receives synchronized access to the updated data.
      */
      template<typename InitialSequenceLoader, typename F>
      void PublishBboQuote(const SecurityBboQuote& bboQuote, int sourceId,
        const InitialSequenceLoader& initialSequenceLoader, const F& f);

      //! Sets a MarketQuote.
      /*!
        \param marketQuote The MarketQuote to set.
        \param sourceId The id of the source setting the value.
        \param initialSequenceLoader Loads initial Sequences for the Security.
        \param f Receives synchronized access to the updated data.
      */
      template<typename InitialSequenceLoader, typename F>
      void PublishMarketQuote(const SecurityMarketQuote& marketQuote,
        int sourceId, const InitialSequenceLoader& initialSequenceLoader,
        const F& f);

      //! Updates a BookQuote.
      /*!
        \param delta The BookQuote storing the change.
        \param sourceId The id of the source setting the value.
        \param initialSequenceLoader Loads initial Sequences for the Security.
        \param f Receives synchronized access to the updated data.
      */
      template<typename InitialSequenceLoader, typename F>
      void UpdateBookQuote(const SecurityBookQuote& delta, int sourceId,
        const InitialSequenceLoader& initialSequenceLoader, const F& f);

      //! Publishes a TimeAndSale.
      /*!
        \param timeAndSale The TimeAndSale to publish.
        \param sourceId The id of the source setting the value.
        \param initialSequenceLoader Loads initial Sequences for the Security.
        \param f Receives synchronized access to the updated data.
      */
      template<typename InitialSequenceLoader, typename F>
      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale,
        int sourceId, const InitialSequenceLoader& initialSequenceLoader,
        const F& f);

      //! Returns an entry's SecurityTechnicals.
      /*!
        \param security The Security whose SecurityTechnicals is to be returned.
        \return A snapshot of the <i>security</i>'s SecurityTechnicals.
      */
      boost::optional<SecurityTechnicals> FindSecurityTechnicals(
        const Security& security);

      //! Returns a Security's real time snapshot.
      /*!
        \param security The Security whose snapshot is to be returned.
        \return The real-time snapshot of the <i>security</i>.
      */
      boost::optional<SecuritySnapshot> FindSnapshot(const Security& security);

      //! Clears market data that originated from a specified source.
      /*!
        \param sourceId The id of the source to clear.
      */
      void Clear(int sourceId);

    private:
      using SyncMarketEntry = Beam::Threading::Sync<MarketEntry>;
      using SyncSecurityEntry = Beam::Threading::Sync<SecurityEntry>;
      Beam::Threading::Sync<rtv::Trie<char, SecurityInfo>> m_securityDatabase;
      Beam::SynchronizedUnorderedMap<MarketCode,
        std::shared_ptr<SyncMarketEntry>> m_marketEntries;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<SyncSecurityEntry>> m_securityEntries;

      template<typename InitialSequenceLoader>
      boost::optional<SyncMarketEntry&> LoadMarketEntry(MarketCode market,
        const InitialSequenceLoader& initialSequenceLoader);
      template<typename InitialSequenceLoader>
      boost::optional<SyncSecurityEntry&> LoadSecurityEntry(
        const Security& security,
        const InitialSequenceLoader& initialSequenceLoader);
  };

  inline MarketDataRegistry::MarketDataRegistry()
      : m_securityDatabase('\0') {}

  inline void MarketDataRegistry::Add(const SecurityInfo& securityInfo) {
    std::string key = ToString(securityInfo.m_security,
      GetDefaultMarketDatabase());
    auto name = boost::to_upper_copy(securityInfo.m_name);
    Beam::Threading::With(m_securityDatabase,
      [&] (rtv::Trie<char, SecurityInfo>& securityDatabase) {
        securityDatabase[key.c_str()] = securityInfo;
        securityDatabase[name.c_str()] = securityInfo;
      });
  }

  inline std::vector<SecurityInfo> MarketDataRegistry::SearchSecurityInfo(
      const std::string& prefix) {
    static const int MAX_MATCH_COUNT = 8;
    std::unordered_set<SecurityInfo> matches;
    auto uppercasePrefix = boost::to_upper_copy(prefix);
    Beam::Threading::With(m_securityDatabase,
      [&] (const rtv::Trie<char, SecurityInfo>& securityDatabase) {
        for(auto i = securityDatabase.startsWith(uppercasePrefix.c_str());
            i != securityDatabase.end(); ++i) {
          matches.insert(*i->second);
        }
      });
    auto i = matches.begin();
    while(i != matches.end()) {
      auto entry = m_securityEntries.FindValue(i->m_security);
      if(!entry.is_initialized()) {
        i = matches.erase(i);
      } else {
        Beam::Threading::With(**entry,
          [&] (SecurityEntry& entry) {
            if((*entry.GetBboQuote())->m_ask.m_price == Money::ZERO) {
              i = matches.erase(i);
            } else {
              ++i;
            }
          });
      }
    }
    std::vector<SecurityInfo> result(matches.begin(), matches.end());
    if(result.size() > MAX_MATCH_COUNT) {
      result.erase(result.begin() + MAX_MATCH_COUNT, result.end());
    }
    return result;
  }

  inline Security MarketDataRegistry::GetPrimaryListing(
      const Security& security) {
    if(security.GetSymbol().empty() ||
        security.GetCountry() == CountryDatabase::NONE) {
      return Security{security.GetSymbol(), CountryDatabase::NONE};
    }
    auto entry = m_securityEntries.Find(security);
    if(!entry.is_initialized()) {
      return Security{security.GetSymbol(), CountryDatabase::NONE};
    }
    return Beam::Threading::With(**entry,
      [&] (SecurityEntry& entry) -> Security {
        if(entry.GetSecurity().GetMarket().IsEmpty()) {
          return Security{security.GetSymbol(), CountryDatabase::NONE};
        }
        return entry.GetSecurity();
      });
  }

  template<typename SecurityInitialSequenceLoader,
    typename MarketInitialSequenceLoader, typename F>
  void MarketDataRegistry::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance, int sourceId,
      const SecurityInitialSequenceLoader& securityInitialSequenceLoader,
      const MarketInitialSequenceLoader& marketInitialSequenceLoader,
      const F& f) {
    auto securityEntry = LoadSecurityEntry(orderImbalance->m_security,
      securityInitialSequenceLoader);
    if(!securityEntry.is_initialized()) {
      return;
    }
    auto marketEntry = LoadMarketEntry(orderImbalance.GetIndex(),
      marketInitialSequenceLoader);
    if(!marketEntry.is_initialized()) {
      return;
    }
    Security security;
    Money referencePrice;
    Beam::Threading::With(*securityEntry,
      [&] (SecurityEntry& securityEntry) {
        security = securityEntry.GetSecurity();
        if(orderImbalance->m_referencePrice == Money::ZERO) {
          if(orderImbalance->m_side == Side::BID) {
            referencePrice = (*securityEntry.GetBboQuote())->m_bid.m_price;
          } else {
            referencePrice = (*securityEntry.GetBboQuote())->m_ask.m_price;
          }
        } else {
          referencePrice = orderImbalance->m_referencePrice;
        }
      });
    Beam::Threading::With(*marketEntry,
      [&] (MarketEntry& entry) {
        OrderImbalance sanitizedOrderImbalance = orderImbalance;
        sanitizedOrderImbalance.m_security = std::move(security);
        sanitizedOrderImbalance.m_referencePrice = std::move(referencePrice);
        auto sequencedOrderImbalance = entry.PublishOrderImbalance(
          std::move(sanitizedOrderImbalance), sourceId);
        if(sequencedOrderImbalance.is_initialized()) {
          f(*sequencedOrderImbalance);
        }
      });
  }

  template<typename InitialSequenceLoader, typename F>
  void MarketDataRegistry::PublishBboQuote(const SecurityBboQuote& bboQuote,
      int sourceId, const InitialSequenceLoader& initialSequenceLoader,
      const F& f) {
    auto entry = LoadSecurityEntry(bboQuote.GetIndex(), initialSequenceLoader);
    if(!entry.is_initialized()) {
      return;
    }
    Beam::Threading::With(*entry,
      [&] (SecurityEntry& entry) {
        if(entry.GetSecurity().GetMarket().IsEmpty()) {
          BEAM_ASSERT(!bboQuote.GetIndex().GetMarket().IsEmpty());
          entry.SetSecurity(bboQuote.GetIndex());
          auto key = ToString(bboQuote.GetIndex(), GetDefaultMarketDatabase());
          SecurityInfo securityInfo(bboQuote.GetIndex(), key, "");
          Beam::Threading::With(m_securityDatabase,
            [&] (rtv::Trie<char, SecurityInfo>& securityDatabase) {
              securityDatabase.insert(key.c_str(), securityInfo);
          });
        }
        auto sequencedBboQuote = entry.PublishBboQuote(std::move(bboQuote),
          sourceId);
        if(sequencedBboQuote.is_initialized()) {
          f(*sequencedBboQuote);
        }
      });
  }

  template<typename InitialSequenceLoader, typename F>
  void MarketDataRegistry::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote, int sourceId,
      const InitialSequenceLoader& initialSequenceLoader, const F& f) {
    auto entry = LoadSecurityEntry(marketQuote.GetIndex(),
      initialSequenceLoader);
    if(!entry.is_initialized()) {
      return;
    }
    Beam::Threading::With(*entry,
      [&] (SecurityEntry& entry) {
        auto sequencedMarketQuote = entry.PublishMarketQuote(
          std::move(marketQuote), sourceId);
        if(sequencedMarketQuote.is_initialized()) {
          f(*sequencedMarketQuote);
        }
      });
  }

  template<typename InitialSequenceLoader, typename F>
  void MarketDataRegistry::UpdateBookQuote(const SecurityBookQuote& delta,
      int sourceId, const InitialSequenceLoader& initialSequenceLoader,
      const F& f) {
    auto entry = LoadSecurityEntry(delta.GetIndex(), initialSequenceLoader);
    if(!entry.is_initialized()) {
      return;
    }
    Beam::Threading::With(*entry,
      [&] (SecurityEntry& entry) {
        auto sequencedBookQuote = entry.UpdateBookQuote(std::move(delta),
          sourceId);
        if(sequencedBookQuote.is_initialized()) {
          f(*sequencedBookQuote);
        }
      });
  }

  template<typename InitialSequenceLoader, typename F>
  void MarketDataRegistry::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale, int sourceId,
      const InitialSequenceLoader& initialSequenceLoader, const F& f) {
    auto entry = LoadSecurityEntry(timeAndSale.GetIndex(),
      initialSequenceLoader);
    if(!entry.is_initialized()) {
      return;
    }
    Beam::Threading::With(*entry,
      [&] (SecurityEntry& entry) {
        auto sequencedTimeAndSale = entry.PublishTimeAndSale(
          std::move(timeAndSale), sourceId);
        if(sequencedTimeAndSale.is_initialized()) {
          f(*sequencedTimeAndSale);
        }
      });
  }

  inline boost::optional<SecurityTechnicals>
      MarketDataRegistry::FindSecurityTechnicals(const Security& security) {
    auto entry = m_securityEntries.Find(security);
    if(!entry.is_initialized()) {
      return boost::optional<SecurityTechnicals>();
    }
    return Beam::Threading::With(**entry,
      [&] (SecurityEntry& entry) -> boost::optional<SecurityTechnicals> {
        return entry.GetSecurityTechnicals();
      });
  }

  inline boost::optional<SecuritySnapshot> MarketDataRegistry::FindSnapshot(
      const Security& security) {
    auto entry = m_securityEntries.Find(security);
    if(!entry.is_initialized()) {
      return boost::optional<SecuritySnapshot>();
    }
    return Beam::Threading::With(**entry,
      [&] (SecurityEntry& entry) -> boost::optional<SecuritySnapshot> {
        return entry.LoadSnapshot();
      });
  }

  inline void MarketDataRegistry::Clear(int sourceId) {
    m_securityEntries.With(
      [&] (const std::unordered_map<
        Security, std::shared_ptr<SyncSecurityEntry>>& securityEntries) {
      for(const auto& entry : securityEntries | boost::adaptors::map_values) {
        Beam::Threading::With(*entry,
          [&] (SecurityEntry& entry) {
            entry.Clear(sourceId);
          });
      }
    });
  }

  template<typename InitialSequenceLoader>
  inline boost::optional<MarketDataRegistry::SyncMarketEntry&>
      MarketDataRegistry::LoadMarketEntry(MarketCode market,
      const InitialSequenceLoader& initialSequenceLoader) {
    if(market == MarketCode()) {
      return boost::optional<SyncMarketEntry&>();
    }
    return *m_marketEntries.GetOrInsert(market,
      [&] {
        auto initialSequences = initialSequenceLoader();
        return std::make_shared<SyncMarketEntry>(market, initialSequences);
      });
  }

  template<typename InitialSequenceLoader>
  boost::optional<MarketDataRegistry::SyncSecurityEntry&>
      MarketDataRegistry::LoadSecurityEntry(const Security& security,
      const InitialSequenceLoader& initialSequenceLoader) {
    if(security.GetSymbol().empty() ||
        security.GetCountry() == CountryDatabase::NONE) {
      return boost::optional<SyncSecurityEntry&>();
    }
    return *m_securityEntries.GetOrInsert(security,
      [&] {
        Security sanitizedSecurity(security.GetSymbol(), security.GetCountry());
        auto initialSequences = initialSequenceLoader();
        return std::make_shared<SyncSecurityEntry>(sanitizedSecurity,
          initialSequences);
      });
  }
}
}

#endif
