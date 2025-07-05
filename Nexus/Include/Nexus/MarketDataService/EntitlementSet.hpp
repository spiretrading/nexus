#ifndef NEXUS_MARKET_DATA_ENTITLEMENT_SET_HPP
#define NEXUS_MARKET_DATA_ENTITLEMENT_SET_HPP
#include <functional>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /** Stores an index into a market data entitlement. */
  struct EntitlementKey {

    /** The market that the data is being provided for. */
    Venue m_venue;

    /** The source of the market data. */
    Venue m_source;

    /** Constructs an EntitlementKey. */
    EntitlementKey() = default;

    /**
     * Constructs an EntitlementKey where the market and source are equal.
     * @param venue The venue that the data is being provided for.
     */
    EntitlementKey(Venue venue);

    /**
     * Constructs an EntitlementKey.
     * @param venue The venue that the data is being provided for.
     * @param source The source of the market data.
     */
    EntitlementKey(Venue venue, Venue source);

    bool operator ==(const EntitlementKey& rhs) const = default;
  };

  inline EntitlementKey::EntitlementKey(Venue venue)
    : EntitlementKey(venue, venue) {}

  inline EntitlementKey::EntitlementKey(Venue venue, Venue source)
    : m_venue(venue),
      m_source(source) {}

  inline std::size_t hash_value(const EntitlementKey& value) {
    auto seed = std::size_t(0);
    boost::hash_combine(seed, value.m_venue);
    boost::hash_combine(seed, value.m_source);
    return seed;
  }
}

namespace std {
  template<>
  struct hash<Nexus::MarketDataService::EntitlementKey> {
    std::size_t operator ()(
        const Nexus::MarketDataService::EntitlementKey& value) const {
      return Nexus::MarketDataService::hash_value(value);
    }
  };
}

namespace Nexus::MarketDataService {

  /** Stores a set of market data entitlements. */
  class EntitlementSet {
    public:

      /** Constructs an empty EntitlementSet. */
      EntitlementSet() = default;

      /**
       * Checks if this session is entitled to a market data message.
       * @param key The EntitlementKey to grant to this session.
       * @param messageType The type of market data message to check.
       * @return <code>true</code> iff this session is entitled to the
       *         <i>messageType</i> on the given <i>market</i>.
       */
      bool HasEntitlement(
        const EntitlementKey& key, MarketDataType messageType) const;

      /**
       * Grants an entitlement to this session.
       * @param key The EntitlementKey to grant to this session.
       * @param messages The messages this session is entitled to on the
       *        specified <i>market</i>.
       */
      void GrantEntitlement(
        const EntitlementKey& key, MarketDataTypeSet messages);

    private:
      std::unordered_map<EntitlementKey, MarketDataTypeSet> m_entitlements;
  };

  /**
   * Checks if an EntitlementSet contains the entitlement needed for a
   * SecurityMarketDataQuery.
   * @param entitlements The EntitlementSet to check.
   * @param query The SecurityMarketDataQuery to validate.
   * @return <code>true</code> iff the <i>entitlements</i> contains the valid
   *         entitlement for the specified <i>query</i>.
   */
  template<typename T>
  bool HasEntitlement(const EntitlementSet& entitlements,
      const SecurityMarketDataQuery& query) {
    return entitlements.HasEntitlement(query.GetIndex().get_venue(),
      GetMarketDataType<T>());
  }

  /**
   * Checks if an EntitlementSet contains the entitlement needed for a
   * MarketWideDataQuery.
   * @param entitlements The EntitlementSet to check.
   * @param query The MarketWideDataQuery to validate.
   * @return <code>true</code> iff the <i>entitlements</i> contains the valid
   *         entitlement for the specified <i>query</i>.
   */
  template<typename T>
  bool HasEntitlement(
      const EntitlementSet& entitlements, const VenueMarketDataQuery& query) {
    return entitlements.HasEntitlement(
      query.GetIndex(), GetMarketDataType<T>());
  }

  inline bool EntitlementSet::HasEntitlement(
      const EntitlementKey& key, MarketDataType messageType) const {
    auto entitlementIterator = m_entitlements.find(key);
    if(entitlementIterator == m_entitlements.end()) {
      entitlementIterator = m_entitlements.find(
        EntitlementKey(Venue(), key.m_source));
      if(entitlementIterator == m_entitlements.end()) {
        return false;
      }
    }
    return entitlementIterator->second.Test(messageType);
  }

  inline void EntitlementSet::GrantEntitlement(
      const EntitlementKey& key, MarketDataTypeSet messages) {
    m_entitlements[key].SetAll(messages);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketDataService::EntitlementKey> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::MarketDataService::EntitlementKey& value, unsigned int version) {
      shuttle.Shuttle("venue", value.m_venue);
      shuttle.Shuttle("source", value.m_source);
    }
  };
}

#endif
