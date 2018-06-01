#ifndef NEXUS_DESTINATION_HPP
#define NEXUS_DESTINATION_HPP
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  //! Represents a destination.
  using Destination = std::string;

  /*! \class DestinationDatabase
      \brief Stores the database of all destinations.
   */
  class DestinationDatabase {
    public:

      /*! \struct Entry
          \brief Stores a single entry in a DestinationDatabase.
       */
      struct Entry {

        //! The id.
        Destination m_id;

        //! The markets this destination is available on.
        std::vector<MarketCode> m_markets;

        //! The destination's description.
        std::string m_description;
      };

      //! Constructs an empty DestinationDatabase.
      DestinationDatabase() = default;

      //! Returns an Entry from its id.
      /*!
        \param id The id of the Entry to lookup.
        \return The Entry with the specified <i>id</i>.
      */
      const Entry& FromId(const std::string& id) const;

      //! Returns the preferred destination for a specified market.
      /*!
        \param market The market to lookup the preferred destination of.
        \return The preferred destination of the specified <i>market</i>.
      */
      const Entry& GetPreferredDestination(MarketCode market) const;

      //! Returns the first Entry matching a predicate.
      /*!
        \param predicate The predicate to match against.
      */
      template<typename P>
      const Entry& SelectEntry(P predicate) const;

      //! Returns all Entries matching a predicate.
      /*!
        \param predicate The predicate to match against.
      */
      template<typename P>
      std::vector<Entry> SelectEntries(P predicate) const;

      //! Returns the manual order entry destination.
      const boost::optional<Entry>& GetManualOrderEntryDestination() const;

      //! Adds an Entry.
      /*!
        \param entry The Entry to add.
      */
      void Add(const Entry& entry);

      //! Sets the manual order entry destination.
      /*!
        \param entry The Entry to set as the manual order entry destination.
      */
      void SetManualOrderEntryDestination(const Entry& entry);

      //! Sets a market's preferred destination.
      /*!
        \param market The market to set.
        \param destination The preferred destination to associate with the
               <i>market</i>.
      */
      void SetPreferredDesintation(MarketCode market,
        const Destination& destination);

      //! Deletes an Entry.
      /*!
        \param destination The id of the Entry to delete.
      */
      void Delete(const Destination& destination);

      //! Deletes a market's preferred destination.
      /*!
        \param market The market whose preferred destination is to be deleted.
      */
      void DeletePreferredDestination(MarketCode market);

    private:
      friend struct Beam::Serialization::Shuttle<DestinationDatabase>;
      static Entry MakeNoneEntry();
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;
      std::unordered_map<MarketCode, Destination> m_preferredDestinations;
      boost::optional<Entry> m_manualOrderEntryDestination;
  };

  //! Parses a DestinationDatabase Entry from a YAML node.
  /*!
    \param node The node to parse the DestinationDatabase Entry from.
    \param marketDatabase The MarketDatabase used to parse MarketCodes.
    \return The DestinationDatabase Entry represented by the <i>node</i>.
  */
  inline DestinationDatabase::Entry ParseDestinationDatabaseEntry(
      const YAML::Node& node, const MarketDatabase& marketDatabase) {
    DestinationDatabase::Entry entry;
    entry.m_id = Beam::Extract<std::string>(node, "id");
    auto marketNames = Beam::Extract<std::vector<std::string>>(node, "markets");
    std::vector<MarketCode> marketCodes;
    for(auto& marketName : marketNames) {
      auto marketCode = ParseMarketCode(marketName, marketDatabase);
      if(marketCode == MarketCode{}) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid market.",
          node.Mark()));
      }
      marketCodes.push_back(marketCode);
    }
    entry.m_markets = marketCodes;
    entry.m_description = Beam::Extract<std::string>(node, "description");
    return entry;
  }

  //! Parses a DestinationDatabase from a YAML node.
  /*!
    \param node The node to parse the DestinationDatabase from.
    \param marketDatabase The MarketDatabase used to parse MarketCodes.
    \return The DestinationDatabase represented by the <i>node</i>.
  */
  inline DestinationDatabase ParseDestinationDatabase(const YAML::Node& node,
      const MarketDatabase& marketDatabase) {
    DestinationDatabase destinationDatabase;
    for(auto entryNode : Beam::GetNode(node, "destinations")) {
      auto entry = ParseDestinationDatabaseEntry(entryNode, marketDatabase);
      destinationDatabase.Add(entry);
    }
    for(auto entryNode : Beam::GetNode(node, "preferred_destinations")) {
      auto market = ParseMarketCode(
        Beam::Extract<std::string>(entryNode, "market"), marketDatabase);
      if(market == MarketCode{}) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid market.",
          entryNode.Mark()));
      }
      auto destination = Beam::Extract<std::string>(entryNode, "destination");
      if(destinationDatabase.FromId(destination).m_id.empty()) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
          "Invalid destination.", entryNode.Mark()));
      }
      destinationDatabase.SetPreferredDesintation(market, destination);
    }
    auto manualOrderEntry = ParseDestinationDatabaseEntry(
      Beam::GetNode(node, "manual_order_entry"), marketDatabase);
    destinationDatabase.SetManualOrderEntryDestination(manualOrderEntry);
    return destinationDatabase;
  }

  //! Tests two DestinationDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two DestinationDatabase Entries are equal.
  */
  inline bool operator ==(const DestinationDatabase::Entry& lhs,
      const DestinationDatabase::Entry& rhs) {
    return lhs.m_id == rhs.m_id && lhs.m_markets == rhs.m_markets &&
      lhs.m_description == rhs.m_description;
  }

  //! Tests two DestinationDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two DestinationDatabase Entries are equal.
  */
  inline bool operator !=(const DestinationDatabase::Entry& lhs,
      const DestinationDatabase::Entry& rhs) {
    return !(lhs == rhs);
  }

  inline const DestinationDatabase::Entry& DestinationDatabase::FromId(
      const std::string& id) const {
    return SelectEntry(
      [&] (const Entry& entry) {
        return entry.m_id == id;
      });
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::GetPreferredDestination(MarketCode market) const {
    auto destinationIterator = m_preferredDestinations.find(market);
    if(destinationIterator == m_preferredDestinations.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
      if(i->m_id == destinationIterator->second) {
        return *i;
      }
    }
    return NoneEntry<void>::NONE_ENTRY;
  }

  template<typename P>
  const DestinationDatabase::Entry& DestinationDatabase::SelectEntry(
      P predicate) const {
    for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
      if(predicate(*i)) {
        return *i;
      }
    }
    return NoneEntry<void>::NONE_ENTRY;
  }

  template<typename P>
  std::vector<DestinationDatabase::Entry> DestinationDatabase::SelectEntries(
      P predicate) const {
    std::vector<DestinationDatabase::Entry> result;
    for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
      if(predicate(*i)) {
        result.push_back(*i);
      }
    }
    return result;
  }

  inline const boost::optional<DestinationDatabase::Entry>&
      DestinationDatabase::GetManualOrderEntryDestination() const {
    return m_manualOrderEntryDestination;
  }

  inline void DestinationDatabase::Add(const Entry& entry) {
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (const Entry& lhs, const Entry& rhs) {
        return lhs.m_id < rhs.m_id;
      });
    if(entryIterator == m_entries.end() || entryIterator->m_id != entry.m_id) {
      m_entries.insert(entryIterator, entry);
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
      [=] (const Entry& entry) {
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

  inline DestinationDatabase::Entry DestinationDatabase::MakeNoneEntry() {
    Entry noneEntry;
    return noneEntry;
  }

  template<typename T>
  DestinationDatabase::Entry DestinationDatabase::NoneEntry<T>::NONE_ENTRY =
    DestinationDatabase::MakeNoneEntry();
}

namespace Beam {
namespace Serialization {
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
}

#endif
