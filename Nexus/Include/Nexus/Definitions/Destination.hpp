#ifndef NEXUS_DESTINATION_HPP
#define NEXUS_DESTINATION_HPP
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  /** Represents a destination. */
  using Destination = std::string;

  /** Stores the database of all destinations. */
  class DestinationDatabase {
    public:

      /** Stores a single entry in a DestinationDatabase. */
      struct Entry {

        /** The id. */
        Destination m_id;

        /** The markets this destination is available on. */
        std::vector<MarketCode> m_markets;

        /** The destination's description. */
        std::string m_description;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** Constructs an empty DestinationDatabase. */
      DestinationDatabase() = default;

      /**
       * Returns an Entry from its id.
       * @param id The id of the Entry to lookup.
       * @return The Entry with the specified <i>id</i>.
       */
      const Entry& FromId(const std::string& id) const;

      /**
       * Returns the preferred destination for a specified market.
       * @param market The market to lookup the preferred destination of.
       * @return The preferred destination of the specified <i>market</i>.
       */
      const Entry& GetPreferredDestination(MarketCode market) const;

      /**
       * Returns the first Entry matching a predicate.
       * @param predicate The predicate to match against.
       */
      template<typename P>
      const Entry& SelectEntry(P&& predicate) const;

      /**
       * Returns all Entries matching a predicate.
       * @param predicate The predicate to match against.
       */
      template<typename P>
      std::vector<Entry> SelectEntries(P&& predicate) const;

      /** Returns the manual order entry destination. */
      const boost::optional<Entry>& GetManualOrderEntryDestination() const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void Add(const Entry& entry);

      /**
       * Sets the manual order entry destination.
       * @param entry The Entry to set as the manual order entry destination.
       */
      void SetManualOrderEntryDestination(const Entry& entry);

      /**
       * Sets a market's preferred destination.
       * @param market The market to set.
       * @param destination The preferred destination to associate with the
       *        <i>market</i>.
       */
      void SetPreferredDesintation(MarketCode market,
        const Destination& destination);

      /**
       * Deletes an Entry.
       * @param destination The id of the Entry to delete.
       */
      void Delete(const Destination& destination);

      /**
       * Deletes a market's preferred destination.
       * @param market The market whose preferred destination is to be deleted.
       */
      void DeletePreferredDestination(MarketCode market);

    private:
      friend struct Beam::Serialization::Shuttle<DestinationDatabase>;
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;
      std::unordered_map<MarketCode, Destination> m_preferredDestinations;
      boost::optional<Entry> m_manualOrderEntryDestination;
  };

  /**
   * Parses a DestinationDatabase Entry from a YAML node.
   * @param node The node to parse the DestinationDatabase Entry from.
   * @param database The MarketDatabase used to parse MarketCodes.
   * @return The DestinationDatabase Entry represented by the <i>node</i>.
   */
  inline DestinationDatabase::Entry ParseDestinationDatabaseEntry(
      const YAML::Node& node, const MarketDatabase& database) {
    return Beam::TryOrNest([&] {
      auto entry = DestinationDatabase::Entry();
      entry.m_id = Beam::Extract<std::string>(node, "id");
      auto names = Beam::Extract<std::vector<std::string>>(node, "markets");
      auto codes = std::vector<MarketCode>();
      for(auto& name : names) {
        auto code = ParseMarketCode(name, database);
        if(code == MarketCode()) {
          BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid market.",
            node.Mark()));
        }
        codes.push_back(code);
      }
      entry.m_markets = codes;
      entry.m_description = Beam::Extract<std::string>(node, "description");
      return entry;
    }, std::runtime_error("Failed to parse destination database entry."));
  }

  /**
   * Parses a DestinationDatabase from a YAML node.
   * @param node The node to parse the DestinationDatabase from.
   * @param database The MarketDatabase used to parse MarketCodes.
   * @return The DestinationDatabase represented by the <i>node</i>.
   */
  inline DestinationDatabase ParseDestinationDatabase(const YAML::Node& node,
      const MarketDatabase& database) {
    return Beam::TryOrNest([&] {
      auto destinationDatabase = DestinationDatabase();
      for(auto node : Beam::GetNode(node, "destinations")) {
        auto entry = ParseDestinationDatabaseEntry(node, database);
        destinationDatabase.Add(entry);
      }
      for(auto node : Beam::GetNode(node, "preferred_destinations")) {
        auto market = ParseMarketCode(
          Beam::Extract<std::string>(node, "market"), database);
        if(market == MarketCode()) {
          BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid market.",
            node.Mark()));
        }
        auto destination = Beam::Extract<std::string>(node, "destination");
        if(destinationDatabase.FromId(destination).m_id.empty()) {
          BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
            "Invalid destination.", node.Mark()));
        }
        destinationDatabase.SetPreferredDesintation(market, destination);
      }
      auto manualOrderEntry = ParseDestinationDatabaseEntry(
        Beam::GetNode(node, "manual_order_entry"), database);
      destinationDatabase.SetManualOrderEntryDestination(manualOrderEntry);
      return destinationDatabase;
    }, std::runtime_error("Failed to parse destination database."));
  }

  inline const DestinationDatabase::Entry& DestinationDatabase::FromId(
      const std::string& id) const {
    return SelectEntry(
      [&] (auto& entry) {
        return entry.m_id == id;
      });
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::GetPreferredDestination(MarketCode market) const {
    auto i = m_preferredDestinations.find(market);
    if(i == m_preferredDestinations.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    auto j = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_id == i->second;
      });
    if(j == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *j;
  }

  template<typename P>
  const DestinationDatabase::Entry& DestinationDatabase::SelectEntry(
      P&& predicate) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      std::forward<P>(predicate));
    if(i == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *i;
  }

  template<typename P>
  std::vector<DestinationDatabase::Entry> DestinationDatabase::SelectEntries(
      P&& predicate) const {
    auto result = std::vector<DestinationDatabase::Entry>();
    std::copy_if(m_entries.begin(), m_entries.end(), std::back_inserter(result),
      std::forward<P>(predicate));
    return result;
  }

  inline const boost::optional<DestinationDatabase::Entry>&
      DestinationDatabase::GetManualOrderEntryDestination() const {
    return m_manualOrderEntryDestination;
  }

  inline void DestinationDatabase::Add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_id < rhs.m_id;
      });
    if(i == m_entries.end() || i->m_id != entry.m_id) {
      m_entries.insert(i, entry);
    }
  }

  inline void DestinationDatabase::SetManualOrderEntryDestination(
      const Entry& entry) {
    m_manualOrderEntryDestination = entry;
  }

  inline void DestinationDatabase::SetPreferredDesintation(MarketCode market,
      const Destination& destination) {
    m_preferredDestinations[market] = destination;
  }

  inline void DestinationDatabase::Delete(const Destination& destination) {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (auto& entry) {
        return entry.m_id == destination;
      });
    if(entryIterator == m_entries.end()) {
      return;
    }
    m_entries.erase(entryIterator);
  }

  inline void DestinationDatabase::DeletePreferredDestination(
      MarketCode market) {
    m_preferredDestinations.erase(market);
  }

  template<typename T>
  DestinationDatabase::Entry DestinationDatabase::NoneEntry<T>::NONE_ENTRY;
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::DestinationDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::DestinationDatabase::Entry& value, unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("markets", value.m_markets);
      shuttle.Shuttle("descriptions", value.m_description);
    }
  };

  template<>
  struct Shuttle<Nexus::DestinationDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::DestinationDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
      shuttle.Shuttle("preferred_destinations", value.m_preferredDestinations);
      shuttle.Shuttle("manual_order_entry_destination",
        value.m_manualOrderEntryDestination);
    }
  };
}

#endif
