#ifndef NEXUS_MARKET_DATA_REGISTRY_HPP
#define NEXUS_MARKET_DATA_REGISTRY_HPP
#include <memory>
#include <string_view>
#include <unordered_set>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Collections/Trie.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"
#include "Nexus/MarketDataService/VenueEntry.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus::MarketDataService {
namespace Details {
  template<typename DataStore>
  Money load_close_price(const Security& security,
      std::string_view market_center, DataStore& data_store) {
    auto query_market_code = Beam::Queries::StringValue(market_center);
    auto market_code_expression =
      Beam::Queries::ConstantExpression(query_market_code);
    auto parameter_expression =
      Beam::Queries::ParameterExpression(0, Nexus::Queries::TimeAndSaleType());
    auto access_expression = Beam::Queries::MemberAccessExpression(
      "market_center", Beam::Queries::StringType(), parameter_expression);
    auto equal_expression = Beam::Queries::MakeEqualsExpression(
      market_code_expression, access_expression);
    auto previous_close_query = MarketDataService::SecurityMarketDataQuery();
    previous_close_query.SetIndex(security);
    previous_close_query.SetRange(Beam::Queries::Range::Total());
    previous_close_query.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    previous_close_query.SetFilter(equal_expression);
    auto result = data_store.load_time_and_sales(previous_close_query);
    if(!result.empty()) {
      return result.back()->m_price;
    }
    return Money::ZERO;
  }
}

  /** Keeps and updates the registry of market data. */
  class MarketDataRegistry {
    public:

      /** Constructs an empty MarketDataRegistry. */
      MarketDataRegistry();

      /**
       * Returns a list of SecurityInfo's matching a prefix.
       * @param prefix The prefix to search for.
       * @return The list of SecurityInfo's that match the <i>prefix</i>.
       */
      std::vector<SecurityInfo>
        search_security_info(const std::string& prefix) const;

      /**
       * Returns a Security's primary listing.
       * @param security The Security to search.
       * @return The <i>security</i>'s primary listing, if no such listing
       *         exists then the MarketCode will be set to empty.
       */
      Security get_primary_listing(const Security& security) const;

      /**
       * Returns a Security's SecurityTechnicals.
       * @param security The Security whose SecurityTechnicals is to be
       *        returned.
       * @return A snapshot of the <i>security</i>'s SecurityTechnicals.
       */
      boost::optional<SecurityTechnicals>
        find_security_technicals(const Security& security) const;

      /**
       * Returns a Security's real time snapshot.
       * @param security The Security whose snapshot is to be returned.
       * @return The real-time snapshot of the <i>security</i>.
       */
      boost::optional<SecuritySnapshot>
        find_snapshot(const Security& security) const;

      /**
       * Adds or updates a SecurityInfo to this registry.
       * @param info The SecurityInfo to add or update.
       */
      void add(const SecurityInfo& info);

      /**
       * Publishes an OrderImbalance.
       * @param imbalance The OrderImbalance to publish.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the market's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void publish(const VenueOrderImbalance& imbalance, int source_id,
        DataStore& data_store, const F& f);

      /**
       * Publishes a BboQuote.
       * @param quote The BboQuote to publish.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void publish(const SecurityBboQuote& quote, int source_id,
        DataStore& data_store, const F& f);

      /**
       * Updates a BookQuote.
       * @param delta The BookQuote storing the change.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void publish(const SecurityBookQuote& delta, int source_id,
        DataStore& data_store, const F& f);

      /**
       * Publishes a TimeAndSale.
       * @param time_and_sale The TimeAndSale to publish.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename DataStore, typename F>
      void publish(const SecurityTimeAndSale& time_and_sale, int source_id,
        DataStore& dataStore, const F& f);

      /**
       * Clears market data that originated from a specified source.
       * @param source_id The id of the source to clear.
       */
      void clear(int source_id);

    private:
      using SyncVenueEntry =
        Beam::Threading::Sync<VenueEntry, Beam::Threading::Mutex>;
      using SyncSecurityEntry =
        Beam::Threading::Sync<SecurityEntry, Beam::Threading::Mutex>;
      Beam::Threading::Sync<rtv::Trie<char, SecurityInfo>> m_security_database;
      Beam::SynchronizedUnorderedSet<Security> m_verified_securities;
      Beam::SynchronizedUnorderedMap<Venue, std::shared_ptr<
        Beam::Remote<SyncVenueEntry, Beam::Threading::Mutex>>> m_market_entries;
      Beam::SynchronizedUnorderedMap<Security, std::shared_ptr<Beam::Remote<
        SyncSecurityEntry, Beam::Threading::Mutex>>> m_security_entries;

      MarketDataRegistry(const MarketDataRegistry&) = delete;
      MarketDataRegistry& operator =(const MarketDataRegistry&) = delete;
      template<typename DataStore>
      boost::optional<SyncVenueEntry&> load(Venue venue, DataStore& data_store);
      template<typename DataStore>
      boost::optional<SyncSecurityEntry&>
        load(const Security& security, DataStore& data_store);
  };

  inline MarketDataRegistry::MarketDataRegistry()
    : m_security_database('\0') {}

  inline std::vector<SecurityInfo> MarketDataRegistry::search_security_info(
      const std::string& prefix) const {
    static const auto MAX_MATCH_COUNT = 8;
    auto matches = std::unordered_set<SecurityInfo>();
    auto uppercase_prefix = boost::to_upper_copy(prefix);
    Beam::Threading::With(m_security_database, [&] (auto& database) {
      for(auto i = database.startsWith(uppercase_prefix.c_str());
          i != database.end(); ++i) {
        matches.insert(*i->second);
      }
    });
    auto i = matches.begin();
    while(i != matches.end()) {
      auto entry = m_security_entries.FindValue(i->m_security);
      if(!entry || !(*entry)->IsAvailable()) {
        i = matches.erase(i);
      } else {
        Beam::Threading::With(***entry, [&] (const auto& entry) {
          if((*entry.get_bbo_quote())->m_ask.m_price == Money::ZERO) {
            i = matches.erase(i);
          } else {
            ++i;
          }
        });
      }
    }
    auto result = std::vector(matches.begin(), matches.end());
    if(result.size() > MAX_MATCH_COUNT) {
      result.erase(result.begin() + MAX_MATCH_COUNT, result.end());
    }
    return result;
  }

  inline Security MarketDataRegistry::get_primary_listing(
      const Security& security) const {
/** TODO
    if(security.get_symbol().empty() || security.get_venue() == Venue()) {
      return Security(security.get_symbol(), Venue());
    }
    if(auto verified_security = m_verified_securities.FindValue(security)) {
      return std::move(*verified_security);
    }
    auto entry = m_security_entries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return Security(security.get_symbol(), Venue());
    }
    return Beam::Threading::With(***entry, [&] (auto& entry) {
      if(entry.get_security().get_venue() == Venue()) {
        return Security(security.get_symbol(), Venue());
      }
      return entry.get_security();
    });
*/
    return security;
  }

  inline boost::optional<SecurityTechnicals>
      MarketDataRegistry::find_security_technicals(
        const Security& security) const {
    auto entry = m_security_entries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return boost::none;
    }
    return Beam::Threading::With(***entry, [&] (const auto& entry) {
      return entry.get_security_technicals();
    });
  }

  inline boost::optional<SecuritySnapshot>
      MarketDataRegistry::find_snapshot(const Security& security) const {
    auto entry = m_security_entries.Find(security);
    if(!entry || !(*entry)->IsAvailable()) {
      return boost::none;
    }
    return Beam::Threading::With(***entry, [&] (const auto& entry) {
      return entry.load_snapshot();
    });
  }

  inline void MarketDataRegistry::add(const SecurityInfo& info) {
    auto key = boost::lexical_cast<std::string>(info.m_security);
    auto name = boost::to_upper_copy(info.m_name);
    Beam::Threading::With(m_security_database, [&] (auto& database) {
      database[key.c_str()] = info;
      database[name.c_str()] = info;
    });
    m_verified_securities.Update(info.m_security);
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::publish(const VenueOrderImbalance& imbalance,
      int source_id, DataStore& data_store, const F& f) {
    auto security_entry = load(imbalance->m_security, data_store);
    if(!security_entry) {
      return;
    }
    auto venue_entry = load(imbalance.GetIndex(), data_store);
    if(!venue_entry) {
      return;
    }
    auto [security, reference_price] =
      Beam::Threading::With(*security_entry, [&] (const auto& entry) {
        auto reference_price = [&] {
          if(imbalance->m_reference_price == Money::ZERO) {
            auto& bbo = **entry.get_bbo_quote();
            return pick(
              imbalance->m_side, bbo.m_ask.m_price, bbo.m_bid.m_price);
          }
          return imbalance->m_reference_price;
        }();
        return std::tuple(entry.get_security(), reference_price);
      });
    Beam::Threading::With(*venue_entry, [&] (auto& entry) {
      auto sanitized_imbalance = OrderImbalance(imbalance);
      sanitized_imbalance.m_security = std::move(security);
      sanitized_imbalance.m_reference_price = std::move(reference_price);
      if(auto sequenced_imbalance =
          entry.publish(std::move(sanitized_imbalance), source_id)) {
        f(*sequenced_imbalance);
      }
    });
  }

  template<typename DataStore, typename F>
  void MarketDataRegistry::publish(const SecurityBboQuote& quote,
      int source_id, DataStore& data_store, const F& f) {
    auto entry = load(quote.GetIndex(), data_store);
    if(!entry) {
      return;
    }
    Beam::Threading::With(*entry, [&] (auto& entry) {
      if(entry.get_security().get_venue() == Venue()) {
        if(auto security = m_verified_securities.FindValue(quote.GetIndex())) {
          entry.set_security(std::move(*security));
        } else {
          entry.set_security(quote.GetIndex());
        }
        auto key = boost::lexical_cast<std::string>(entry.get_security());
        auto info = SecurityInfo(entry.get_security(), key, "", 0);
        Beam::Threading::With(m_security_database, [&] (auto& database) {
          database.insert(key.c_str(), info);
        });
      }
      if(auto sequenced_quote = entry.publish(std::move(quote), source_id)) {
        f(*sequenced_quote);
      }
    });
  }

#if 0
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

  inline void MarketDataRegistry::Clear(int sourceId) {
    auto entries = std::vector<std::shared_ptr<
      Beam::Remote<SyncSecurityEntry, Beam::Threading::Mutex>>>();
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
      return std::make_shared<
        Beam::Remote<SyncMarketEntry, Beam::Threading::Mutex>>(
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
      return std::make_shared<
        Beam::Remote<SyncSecurityEntry, Beam::Threading::Mutex>>(
          [&] (auto& entry) {
            auto sanitizedSecurity = GetPrimaryListing(security);
            auto initialSequences = LoadInitialSequences(dataStore,
              sanitizedSecurity);
            auto closePrice = Details::LoadClosePrice(sanitizedSecurity,
              dataStore);
            entry.emplace(sanitizedSecurity, closePrice, initialSequences);
          });
    });
    return **entry;
  }
#endif
}

#endif
