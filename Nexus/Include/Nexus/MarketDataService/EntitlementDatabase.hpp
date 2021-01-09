#ifndef NEXUS_MARKET_DATA_ENTITLEMENT_DATABASE_HPP
#define NEXUS_MARKET_DATA_ENTITLEMENT_DATABASE_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"

namespace Nexus::MarketDataService {

  /** Stores the database of all market data entitlements. */
  class EntitlementDatabase {
    public:

      /** Stores a single entry in an EntitlementDatabase. */
      struct Entry {

        /** The name the entitlement displays as. */
        std::string m_name;

        /** The price of the entitlement. */
        Money m_price;

        /** The currency the entitlement is charged in. */
        CurrencyId m_currency;

        /**
         * The entitlement's group entry, basically it's the directory all
         * accounts with this entitlement are located under.
         */
        Beam::ServiceLocator::DirectoryEntry m_groupEntry;

        /**
         * Stores the entitlement's applicability.  Each market this
         * entitlement applies to is mapped to the message types the
         * entitlement grants.
         */
        std::unordered_map<EntitlementKey, MarketDataTypeSet> m_applicability;
      };

      /** Constructs an empty MarketDatabase. */
      EntitlementDatabase() = default;

      /** Returns all Entries. */
      const std::vector<Entry>& GetEntries() const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void Add(const Entry& entry);

      /**
       * Deletes an Entry.
       * @param groupEntry The Entry's group to delete.
       */
      void Delete(const Beam::ServiceLocator::DirectoryEntry& groupEntry);

    private:
      friend struct Beam::Serialization::DataShuttle;
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline const std::vector<EntitlementDatabase::Entry>&
      EntitlementDatabase::GetEntries() const {
    return m_entries;
  }

  inline void EntitlementDatabase::Add(const Entry& entry) {
    m_entries.push_back(entry);
  }

  inline void EntitlementDatabase::Delete(
      const Beam::ServiceLocator::DirectoryEntry& groupEntry) {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const auto& entry) {
        return entry.m_groupEntry == groupEntry;
      });
    if(entryIterator == m_entries.end()) {
      return;
    }
    m_entries.erase(entryIterator);
  }

  template<typename Shuttler>
  void EntitlementDatabase::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("entries", m_entries);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketDataService::EntitlementDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::MarketDataService::EntitlementDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("price", value.m_price);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("group_entry", value.m_groupEntry);
      shuttle.Shuttle("applicability", value.m_applicability);
    }
  };
}

#endif
