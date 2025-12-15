#ifndef NEXUS_MARKET_DATA_ENTITLEMENT_DATABASE_HPP
#define NEXUS_MARKET_DATA_ENTITLEMENT_DATABASE_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"

namespace Nexus {

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
        Beam::DirectoryEntry m_group_entry;

        /**
         * Stores the entitlement's applicability.  Each market this entitlement
         * applies to is mapped to the message types the entitlement grants.
         */
        std::unordered_map<EntitlementKey, MarketDataTypeSet> m_applicability;

        bool operator ==(const Entry&) const = default;
      };

      /** The entry returned for any failed lookup. */
      inline static const auto NONE = Entry();

      /** Constructs an empty EntitlementDatabase. */
      EntitlementDatabase() = default;

      EntitlementDatabase(const EntitlementDatabase& database) noexcept;

      /** Returns all Entries. */
      Beam::View<const Entry> get_entries() const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Removes an Entry.
       * @param group The Entry's group to delete.
       */
      void remove(const Beam::DirectoryEntry& group);

      EntitlementDatabase& operator =(
        const EntitlementDatabase& database) noexcept;

    private:
      friend struct Beam::Shuttle<EntitlementDatabase>;
      std::atomic<std::shared_ptr<std::vector<Entry>>> m_entries;
  };

  inline EntitlementDatabase::EntitlementDatabase(
    const EntitlementDatabase& database) noexcept
    : m_entries(database.m_entries.load()) {}

  inline Beam::View<const EntitlementDatabase::Entry>
      EntitlementDatabase::get_entries() const {
    if(auto entries = m_entries.load()) {
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline void EntitlementDatabase::add(const Entry& entry) {
    while(true) {
      auto entries = m_entries.load();
      auto new_entries = [&] {
        if(!entries) {
          auto new_entries = std::make_shared<std::vector<Entry>>();
          new_entries->push_back(entry);
          return new_entries;
        }
        auto i = std::find_if(entries->begin(), entries->end(),
          [&] (const auto& i) {
            return entry.m_group_entry == i.m_group_entry;
          });
        if(i == entries->end()) {
          auto new_entries = std::make_shared<std::vector<Entry>>(*entries);
          new_entries->insert(
            new_entries->begin() + std::distance(entries->begin(), i), entry);
          return new_entries;
        }
        return std::shared_ptr<std::vector<Entry>>();
      }();
      if(!new_entries ||
          m_entries.compare_exchange_weak(entries, new_entries)) {
        break;
      }
    }
  }

  inline void EntitlementDatabase::remove(const Beam::DirectoryEntry& group) {
    while(true) {
      auto entries = m_entries.load();
      if(!entries) {
        break;
      }
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& i) {
          return group == i.m_group_entry;
        });
      if(i != entries->end()) {
        auto new_entries = std::make_shared<std::vector<Entry>>(*entries);
        new_entries->erase(
          new_entries->begin() + std::distance(entries->begin(), i));
        if(m_entries.compare_exchange_weak(entries, new_entries)) {
          break;
        }
      } else {
        break;
      }
    }
  }

  inline EntitlementDatabase& EntitlementDatabase::operator =(
      const EntitlementDatabase& database) noexcept {
    m_entries.store(database.m_entries.load());
    return *this;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::EntitlementDatabase::Entry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::EntitlementDatabase::Entry& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("price", value.m_price);
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("group_entry", value.m_group_entry);
      shuttle.shuttle("applicability", value.m_applicability);
    }
  };

  template<>
  struct Shuttle<Nexus::EntitlementDatabase> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::EntitlementDatabase& value,
        unsigned int version) const {
      if constexpr(IsSender<S>) {
        if(auto entries = value.m_entries.load()) {
          shuttle.send("entries", *entries);
        }
      } else {
        auto entries =
          std::make_shared<std::vector<Nexus::EntitlementDatabase::Entry>>();
        shuttle.shuttle("entries", *entries);
        value.m_entries.store(std::move(entries));
      }
    }
  };
}

#endif
