#ifndef NEXUS_MARKET_DATA_REGISTRY_HPP
#define NEXUS_MARKET_DATA_REGISTRY_HPP
#include <functional>
#include <memory>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <tsl/htrie_map.h>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"
#include "Nexus/MarketDataService/VenueEntry.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus::Details {
  Money load_close_price(
      const Security& security, const std::string& market_center,
      IsHistoricalDataStore auto& data_store) {
    auto previous_close_query = SecurityMarketDataQuery();
    previous_close_query.set_index(security);
    previous_close_query.set_range(Beam::Range::TOTAL);
    previous_close_query.set_snapshot_limit(Beam::SnapshotLimit::Type::TAIL, 1);
    previous_close_query.set_filter(market_center ==
      TimeAndSaleAccessor::from_parameter(0).get_market_center());
    auto result = data_store.load_time_and_sales(previous_close_query);
    if(!result.empty()) {
      return result.back()->m_price;
    }
    return Money::ZERO;
  }

  struct PrimaryListingKey {
    std::string m_symbol;
    boost::variant<Venue, CountryCode> m_region;

    bool operator ==(const PrimaryListingKey&) const = default;
  };
}

namespace std {
  template<>
  struct hash<Nexus::Details::PrimaryListingKey> {
    std::size_t operator ()(
        const Nexus::Details::PrimaryListingKey& key) const noexcept {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, key.m_symbol);
      boost::hash_combine(seed, key.m_region);
      return seed;
    }
  };
}

namespace Nexus {

  /** Keeps and updates the registry of market data. */
  class MarketDataRegistry {
    public:

      /**
       * Constructs an empty MarketDataRegistry.
       * @param venues The venues used to resolve primary listings.
       * @param time_zones The database of time zones.
       */
      MarketDataRegistry(VenueDatabase venues,
        boost::local_time::tz_database time_zones) noexcept;

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
      template<typename F>
      void publish(const VenueOrderImbalance& imbalance, int source_id,
        IsHistoricalDataStore auto& data_store, const F& f);

      /**
       * Publishes a BboQuote.
       * @param quote The BboQuote to publish.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename F>
      void publish(const SecurityBboQuote& quote, int source_id,
        IsHistoricalDataStore auto& data_store, const F& f);

      /**
       * Updates a BookQuote.
       * @param delta The BookQuote storing the change.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename F>
      void publish(const SecurityBookQuote& delta, int source_id,
        IsHistoricalDataStore auto& data_store, const F& f);

      /**
       * Publishes a TimeAndSale.
       * @param time_and_sale The TimeAndSale to publish.
       * @param source_id The id of the source setting the value.
       * @param data_store Used to initialize the Security's data.
       * @param f Receives synchronized access to the updated data.
       */
      template<typename F>
      void publish(const SecurityTimeAndSale& time_and_sale, int source_id,
        IsHistoricalDataStore auto& data_store, const F& f);

      /**
       * Clears market data that originated from a specified source.
       * @param source_id The id of the source to clear.
       */
      void clear(int source_id);

    private:
      using PrimaryListingKey = Details::PrimaryListingKey;
      template<typename> friend struct std::hash;
      using SyncVenueEntry = Beam::Sync<VenueEntry, Beam::Mutex>;
      using SyncSecurityEntry = Beam::Sync<SecurityEntry, Beam::Mutex>;
      VenueDatabase m_venues;
      boost::local_time::tz_database m_time_zones;
      Beam::Sync<tsl::htrie_map<char, SecurityInfo>> m_security_database;
      Beam::SynchronizedUnorderedMap<PrimaryListingKey, Security>
        m_primary_listings;
      Beam::SynchronizedUnorderedMap<Venue, std::shared_ptr<
        Beam::Remote<SyncVenueEntry, Beam::Mutex>>> m_venue_entries;
      Beam::SynchronizedUnorderedMap<Security, std::shared_ptr<Beam::Remote<
        SyncSecurityEntry, Beam::Mutex>>> m_security_entries;

      MarketDataRegistry(const MarketDataRegistry&) = delete;
      MarketDataRegistry& operator =(const MarketDataRegistry&) = delete;
      boost::optional<SyncVenueEntry&> load(
        Venue venue, IsHistoricalDataStore auto& data_store);
      boost::optional<SyncSecurityEntry&> load(
        const Security& security, IsHistoricalDataStore auto & data_store);
  };

  inline MarketDataRegistry::MarketDataRegistry(
    VenueDatabase venues, boost::local_time::tz_database time_zones) noexcept
    : m_venues(std::move(venues)),
      m_time_zones(std::move(time_zones)) {}

  inline std::vector<SecurityInfo> MarketDataRegistry::search_security_info(
      const std::string& prefix) const {
    auto matches = std::unordered_set<SecurityInfo>();
    auto uppercase_prefix = boost::to_upper_copy(prefix);
    Beam::with(m_security_database, [&] (auto& database) {
      auto range = database.equal_prefix_range(uppercase_prefix);
      for(auto i = range.first; i != range.second; ++i) {
        matches.insert(*i);
      }
    });
    auto activity_result = std::vector<std::pair<Money, SecurityInfo>>();
    for(auto& match : matches) {
      activity_result.push_back(std::pair(Money::ZERO, match));
    }
    for(auto& entry : activity_result) {
      if(auto technicals = find_security_technicals(entry.second.m_security)) {
        entry.first = abs(technicals->m_volume * technicals->m_open);
      }
    }
    std::sort(activity_result.begin(), activity_result.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
      });
    auto result = std::vector<SecurityInfo>();
    for(auto& entry : activity_result) {
      result.push_back(std::move(entry.second));
      static const auto MAX_MATCH_COUNT = 8;
      if(result.size() >= MAX_MATCH_COUNT) {
        break;
      }
    }
    return result;
  }

  inline Security MarketDataRegistry::get_primary_listing(
      const Security& security) const {
    if(security.get_symbol().empty() || !security.get_venue()) {
      return Security(security.get_symbol(), Venue());
    }
    auto venue_key =
      PrimaryListingKey(security.get_symbol(), security.get_venue());
    if(auto verified_security = m_primary_listings.try_load(venue_key)) {
      return *verified_security;
    }
    auto& venue_entry = m_venues.from(security.get_venue());
    if(!venue_entry.m_venue) {
      return security;
    }
    auto country_key = 
      PrimaryListingKey(security.get_symbol(), venue_entry.m_country_code);
    if(auto verified_security = m_primary_listings.try_load(country_key)) {
      const_cast<MarketDataRegistry*>(this)->m_primary_listings.insert(
        venue_key, *verified_security);
      return *verified_security;
    }
    return security;
  }

  inline boost::optional<SecurityTechnicals>
      MarketDataRegistry::find_security_technicals(
        const Security& security) const {
    auto entry = m_security_entries.find(get_primary_listing(security));
    if(!entry || !(*entry)->is_available()) {
      return boost::none;
    }
    return Beam::with(***entry, [&] (const auto& entry) {
      return entry.get_security_technicals();
    });
  }

  inline boost::optional<SecuritySnapshot>
      MarketDataRegistry::find_snapshot(const Security& security) const {
    auto entry = m_security_entries.find(get_primary_listing(security));
    if(!entry || !(*entry)->is_available()) {
      return boost::none;
    }
    return Beam::with(***entry, [&] (const auto& entry) {
      return entry.load_snapshot();
    });
  }

  inline void MarketDataRegistry::add(const SecurityInfo& info) {
    auto key = boost::lexical_cast<std::string>(info.m_security);
    auto name = boost::to_upper_copy(info.m_name);
    Beam::with(m_security_database, [&] (auto& database) {
      database[key] = info;
      database[name] = info;
    });
    auto& venue_entry = m_venues.from(info.m_security.get_venue());
    if(!venue_entry.m_venue) {
      return;
    }
    auto venue_key = PrimaryListingKey(
      info.m_security.get_symbol(), info.m_security.get_venue());
    m_primary_listings.update(venue_key, info.m_security);
    auto country_key = PrimaryListingKey(
      info.m_security.get_symbol(), venue_entry.m_country_code);
    m_primary_listings.update(country_key, info.m_security);
  }

  template<typename F>
  void MarketDataRegistry::publish(const VenueOrderImbalance& imbalance,
      int source_id, IsHistoricalDataStore auto& data_store, const F& f) {
    auto security_entry = load(imbalance->m_security, data_store);
    if(!security_entry) {
      return;
    }
    auto venue_entry = load(imbalance.get_index(), data_store);
    if(!venue_entry) {
      return;
    }
    auto [security, reference_price] =
      Beam::with(*security_entry, [&] (const auto& entry) {
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
    Beam::with(*venue_entry, [&] (auto& entry) {
      auto sanitized_imbalance = OrderImbalance(imbalance);
      sanitized_imbalance.m_security = std::move(security);
      sanitized_imbalance.m_reference_price = std::move(reference_price);
      if(auto sequenced_imbalance =
          entry.publish(std::move(sanitized_imbalance), source_id)) {
        f(*sequenced_imbalance);
      }
    });
  }

  template<typename F>
  void MarketDataRegistry::publish(const SecurityBboQuote& quote,
      int source_id, IsHistoricalDataStore auto& data_store, const F& f) {
    auto entry = load(quote.get_index(), data_store);
    if(!entry) {
      return;
    }
    Beam::with(*entry, [&] (auto& entry) {
      if(auto sequenced_quote = entry.publish(quote, source_id)) {
        f(*sequenced_quote);
      }
    });
  }

  template<typename F>
  void MarketDataRegistry::publish(const SecurityBookQuote& delta,
      int source_id, IsHistoricalDataStore auto& data_store, const F& f) {
    auto entry = load(delta.get_index(), data_store);
    if(!entry) {
      return;
    }
    Beam::with(*entry, [&] (auto& entry) {
      if(auto sequenced_quote = entry.publish(delta, source_id)) {
        f(*sequenced_quote);
      }
    });
  }

  template<typename F>
  void MarketDataRegistry::publish(const SecurityTimeAndSale& time_and_sale,
      int source_id, IsHistoricalDataStore auto& data_store, const F& f) {
    auto entry = load(time_and_sale.get_index(), data_store);
    if(!entry) {
      return;
    }
    Beam::with(*entry, [&] (auto& entry) {
      if(auto sequenced_time_and_sale =
          entry.publish(time_and_sale, source_id)) {
        f(*sequenced_time_and_sale);
      }
    });
  }

  inline void MarketDataRegistry::clear(int source_id) {
    auto entries = std::vector<
      std::shared_ptr<Beam::Remote<SyncSecurityEntry, Beam::Mutex>>>();
    m_security_entries.with([&] (auto& security_entries) {
      for(auto& entry : security_entries | std::views::values) {
        entries.push_back(entry);
      }
    });
    for(auto& entry : entries) {
      if(entry->is_available()) {
        Beam::with(**entry, [&] (auto& entry) {
          entry.clear(source_id);
        });
      }
    }
  }

  boost::optional<MarketDataRegistry::SyncVenueEntry&> MarketDataRegistry::load(
      Venue venue, IsHistoricalDataStore auto& data_store) {
    if(!venue) {
      return boost::none;
    }
    auto entry = m_venue_entries.get_or_insert(venue, [&] {
      return std::make_shared<Beam::Remote<SyncVenueEntry, Beam::Mutex>>(
        [&] (auto& entry) {
          entry.emplace(venue, load_initial_sequences(data_store, venue));
        });
    });
    return **entry;
  }

  boost::optional<MarketDataRegistry::SyncSecurityEntry&>
      MarketDataRegistry::load(
        const Security& security, IsHistoricalDataStore auto& data_store) {
    if(security.get_symbol().empty() || !security.get_venue()) {
      return boost::none;
    }
    auto entry = m_security_entries.get_or_insert(security, [&] {
      return std::make_shared<
        Beam::Remote<SyncSecurityEntry, Beam::Mutex>>([&] (auto& entry) {
          auto sanitized_security = get_primary_listing(security);
          auto initial_sequences =
            load_initial_sequences(data_store, sanitized_security);
          auto& market_center =
            m_venues.from(sanitized_security.get_venue()).m_market_center;
          auto close = Details::load_close_price(
            sanitized_security, market_center, data_store);
          entry.emplace(sanitized_security, m_venues, m_time_zones, close,
            initial_sequences);
        });
    });
    return **entry;
  }
}

#endif
