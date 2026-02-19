#ifndef NEXUS_MARKET_DATA_ENTITLEMENT_SET_HPP
#define NEXUS_MARKET_DATA_ENTITLEMENT_SET_HPP
#include <functional>
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/MarketDataService/TickerQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Stores an index into a market data entitlement. */
  struct EntitlementKey {

    /** The market that the data is being provided for. */
    Venue m_venue;

    /** The source of the market data. */
    Venue m_source;

    /** Constructs an empty EntitlementKey. */
    EntitlementKey() = default;

    /**
     * Constructs an EntitlementKey where the venue and source are equal.
     * @param venue The venue that the data is being provided for.
     */
    EntitlementKey(Venue venue);

    /**
     * Constructs an EntitlementKey.
     * @param venue The venue that the data is being provided for.
     * @param source The source of the market data.
     */
    EntitlementKey(Venue venue, Venue source);

    bool operator ==(const EntitlementKey&) const = default;
  };

  inline EntitlementKey::EntitlementKey(Venue venue)
    : EntitlementKey(venue, venue) {}

  inline EntitlementKey::EntitlementKey(Venue venue, Venue source)
    : m_venue(std::move(venue)),
      m_source(std::move(source)) {}

  inline std::ostream& operator <<(
      std::ostream& out, const EntitlementKey& key) {
    return out << '(' << key.m_venue << ", " << key.m_source << ')';
  }
}

namespace std {
  template<>
  struct hash<Nexus::EntitlementKey> {
    std::size_t operator ()(const Nexus::EntitlementKey& value) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, std::hash<Nexus::Venue>()(value.m_venue));
      boost::hash_combine(seed, std::hash<Nexus::Venue>()(value.m_source));
      return seed;
    }
  };
}

namespace Nexus {

  /** Stores a set of market data entitlements. */
  class EntitlementSet {
    public:

      /** Constructs an empty EntitlementSet. */
      EntitlementSet() = default;

      /**
       * Checks if this session is entitled to a market data message.
       * @param key The EntitlementKey to grant to this session.
       * @param type The type of market data message to check.
       * @return <code>true</code> iff this session is entitled to the
       *         <i>type</i> on the given <i>key</i>.
       */
      bool contains(const EntitlementKey& key, MarketDataType type) const;

      /**
       * Grants an entitlement to this session.
       * @param key The EntitlementKey to grant to this session.
       * @param messages The messages this session is entitled to on the
       *        specified <i>market</i>.
       */
      void grant(const EntitlementKey& key, MarketDataTypeSet messages);

      bool operator ==(const EntitlementSet&) const = default;

    private:
      std::unordered_map<EntitlementKey, MarketDataTypeSet> m_entitlements;
  };

  /**
   * Checks if an EntitlementSet contains the entitlement needed for a
   * TickerQuery.
   * @param entitlements The EntitlementSet to check.
   * @param query The TickerQuery to validate.
   * @return <code>true</code> iff the <i>entitlements</i> contains the valid
   *         entitlement for the specified <i>query</i>.
   */
  template<typename T>
  bool contains(const EntitlementSet& entitlements, const TickerQuery& query) {
    return entitlements.contains(
      query.get_index().get_venue(), get_market_data_type<T>());
  }

  /**
   * Checks if an EntitlementSet contains the entitlement needed for a
   * VenueMarketDataQuery.
   * @param entitlements The EntitlementSet to check.
   * @param query The VenueMarketDataQuery to validate.
   * @return <code>true</code> iff the <i>entitlements</i> contains the valid
   *         entitlement for the specified <i>query</i>.
   */
  template<typename T>
  bool contains(
      const EntitlementSet& entitlements, const VenueMarketDataQuery& query) {
    return entitlements.contains(query.get_index(), get_market_data_type<T>());
  }

  inline bool EntitlementSet::contains(
      const EntitlementKey& key, MarketDataType type) const {
    auto i = m_entitlements.find(key);
    if(i == m_entitlements.end()) {
      i = m_entitlements.find(EntitlementKey(Venue(), key.m_source));
      if(i == m_entitlements.end()) {
        return false;
      }
    }
    return i->second.test(type);
  }

  inline void EntitlementSet::grant(
      const EntitlementKey& key, MarketDataTypeSet messages) {
    m_entitlements[key].set(messages);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::EntitlementKey> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::EntitlementKey& value, unsigned int version) const {
      shuttle.shuttle("venue", value.m_venue);
      shuttle.shuttle("source", value.m_source);
    }
  };
}

#endif
