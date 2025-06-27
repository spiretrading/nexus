#ifndef NEXUS_DESTINATION_HPP
#define NEXUS_DESTINATION_HPP
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Venue.hpp"

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

        /** The venues this destination is available on. */
        std::vector<Venue> m_venues;

        /** The destination's description. */
        std::string m_description;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** The entry returned for any failed lookup. */
      inline static const auto NONE = Entry();

      /** Constructs an empty DestinationDatabase. */
      DestinationDatabase() = default;

      DestinationDatabase(const DestinationDatabase& database) noexcept;

      /** Returns the list of currencies represented. */
      Beam::View<const Entry> get_entries() const;

      /**
       * Returns an Entry from its id.
       * @param id The id of the Entry to lookup.
       * @return The Entry with the specified <i>id</i>.
       */
      const Entry& from(const Destination& id) const;

      /**
       * Returns the preferred destination for a specified venue.
       * @param venue The venue to lookup the preferred destination of.
       * @return The preferred destination of the specified <i>venue</i>.
       */
      const Entry& get_preferred_destination(Venue venue) const;

      /**
       * Returns the first Entry matching a predicate.
       * @param predicate The predicate to match against.
       */
      template<typename P>
      const Entry& select_first(P&& predicate) const;

      /**
       * Returns all Entries matching a predicate.
       * @param predicate The predicate to match against.
       */
      template<typename P>
      std::vector<Entry> select_all(P&& predicate) const;

      /** Returns the manual order entry destination. */
      const boost::optional<Entry>& get_manual_order_entry_destination() const;

      /**
       * Sets the manual order entry destination.
       * @param entry The Entry to set as the manual order entry destination.
       */
      void set_manual_order_entry_destination(const Entry& entry);

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Sets a venue's preferred destination.
       * @param venue The venue to set.
       * @param destination The preferred destination to associate with the
       *        <i>venue</i>.
       */
      void set_preferred_desintation(
        Venue venue, const Destination& destination);

      /**
       * Removes an Entry.
       * @param destination The id of the Entry to delete.
       */
      void remove(const Destination& destination);

      /**
       * Deletes a venue's preferred destination.
       * @param venue The venue whose preferred destination is to be deleted.
       */
      void remove_preferred_destination(Venue venue);

      DestinationDatabase& operator =(
        const DestinationDatabase& database) noexcept;

    private:
      friend struct Beam::Serialization::Shuttle<DestinationDatabase>;
      struct Data {
        std::vector<Entry> entries;
        std::unordered_map<Venue, Destination> preferred_destinations;
        boost::optional<Entry> manual_order_entry_destination;
      };
      std::atomic<std::shared_ptr<Data>> m_data{std::make_shared<Data>()};  };

  /**
   * Parses a DestinationDatabase Entry from a YAML node.
   * @param node The node to parse the DestinationDatabase Entry from.
   * @param database The VenueDatabase used to parse Venues.
   * @return The DestinationDatabase Entry represented by the <i>node</i>.
   */
  inline DestinationDatabase::Entry parse_destination_database_entry(
      const YAML::Node& node, const VenueDatabase& database) {
    return Beam::TryOrNest([&] {
      auto entry = DestinationDatabase::Entry();
      entry.m_id = Beam::Extract<std::string>(node, "id");
      auto names = Beam::Extract<std::vector<std::string>>(node, "venues");
      for(auto& name : names) {
        auto venue = parse_venue(name, database);
        if(venue == Venue()) {
          BOOST_THROW_EXCEPTION(
            Beam::MakeYamlParserException("Invalid venue.", node.Mark()));
        }
        entry.m_venues.push_back(venue);
      }
      entry.m_description = Beam::Extract<std::string>(node, "description");
      return entry;
    }, std::runtime_error("Failed to parse destination database entry."));
  }

  /**
   * Parses a DestinationDatabase from a YAML node.
   * @param node The node to parse the DestinationDatabase from.
   * @param database The VenueDatabase used to parse Venues.
   * @return The DestinationDatabase represented by the <i>node</i>.
   */
  inline DestinationDatabase parse_destination_database(
      const YAML::Node& node, const VenueDatabase& database) {
    return Beam::TryOrNest([&] {
      auto destination_database = DestinationDatabase();
      for(auto node : Beam::GetNode(node, "destinations")) {
        auto entry = parse_destination_database_entry(node, database);
        destination_database.add(entry);
      }
      for(auto node : Beam::GetNode(node, "preferred_destinations")) {
        auto venue =
          parse_venue(Beam::Extract<std::string>(node, "venue"), database);
        if(venue == Venue()) {
          BOOST_THROW_EXCEPTION(
            Beam::MakeYamlParserException("Invalid venue.", node.Mark()));
        }
        auto destination = Beam::Extract<std::string>(node, "destination");
        if(destination_database.from(destination).m_id.empty()) {
          BOOST_THROW_EXCEPTION(
            Beam::MakeYamlParserException("Invalid destination.", node.Mark()));
        }
        destination_database.set_preferred_desintation(venue, destination);
      }
      auto manual_order_entry = parse_destination_database_entry(
        Beam::GetNode(node, "manual_order_entry"), database);
      destination_database.set_manual_order_entry_destination(
        manual_order_entry);
      return destination_database;
    }, std::runtime_error("Failed to parse destination database."));
  }

  inline DestinationDatabase::DestinationDatabase(
    const DestinationDatabase& other) noexcept
    : m_data(other.m_data.load()) {}

  inline Beam::View<const DestinationDatabase::Entry>
      DestinationDatabase::get_entries() const {
    if(auto data = m_data.load()) {
      auto entries = std::shared_ptr<std::vector<Entry>>(data, &data->entries);
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::from(const Destination& id) const {
    if(auto data = m_data.load()) {
      auto i = std::lower_bound(data->entries.begin(), data->entries.end(), id,
        [] (auto const& lhs, auto const& rhs) {
          return lhs.m_id < rhs;
        });
      if(i != data->entries.end() && i->m_id == id) {
        return *i;
      }
    }
    return NONE;
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::get_preferred_destination(Venue venue) const {
    if(auto data = m_data.load()) {
      auto i = data->preferred_destinations.find(venue);
      if(i != data->preferred_destinations.end()) {
        return from(i->second);
      }
    }
    return NONE;
  }

  template<typename P>
  inline const DestinationDatabase::Entry&
      DestinationDatabase::select_first(P&& predicate) const {
    if(auto data = m_data.load()) {
      for(auto& entry : data->entries) {
        if(predicate(entry)) {
          return entry;
        }
      }
    }
    return NONE;
  }

  template<typename P>
  inline std::vector<DestinationDatabase::Entry>
      DestinationDatabase::select_all(P&& predicate) const {
    auto result = std::vector<Entry>();
    if(auto data = m_data.load()) {
      for(auto& entry : data->entries) {
        if(predicate(entry)) {
          result.push_back(entry);
        }
      }
    }
    return result;
  }

  inline const boost::optional<DestinationDatabase::Entry>&
      DestinationDatabase::get_manual_order_entry_destination() const {
    if(auto data = m_data.load()) {
      return data->manual_order_entry_destination;
    }
    static const auto NONE = boost::optional<DestinationDatabase::Entry>();
    return NONE;
  }

  inline void DestinationDatabase::set_manual_order_entry_destination(
      const Entry& entry) {
    while(true) {
      auto old = m_data.load();
      auto ne = std::make_shared<Data>(*old);
      ne->manual_order_entry_destination = entry;
      if(m_data.compare_exchange_weak(old, ne)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::add(const Entry& entry) {
    while(true) {
      auto old = m_data.load();
      auto ne = std::make_shared<Data>(*old);
      auto i = std::lower_bound(
        ne->entries.begin(), ne->entries.end(), entry,
        [] (auto const& lhs, auto const& rhs) {
          return lhs.m_id < rhs.m_id;
        });
      if(i == ne->entries.end() || i->m_id != entry.m_id) {
        ne->entries.insert(
          ne->entries.begin() + std::distance(ne->entries.begin(), i), entry);
      } else {
        return;
      }
      if(m_data.compare_exchange_weak(old, ne)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::set_preferred_desintation(
      Venue venue, const Destination& destination) {
    while(true) {
      auto old = m_data.load();
      auto ne = std::make_shared<Data>(*old);
      ne->preferred_destinations[venue] = destination;
      if(m_data.compare_exchange_weak(old, ne)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::remove(const Destination& id) {
    while(true) {
      auto old = m_data.load();
      auto ne = std::make_shared<Data>(*old);
      auto i = std::lower_bound(
        ne->entries.begin(), ne->entries.end(), id,
        [] (auto const& lhs, auto const& rhs) {
          return lhs.m_id < rhs;
        });
      if(i != ne->entries.end() && i->m_id == id) {
        ne->entries.erase(
          ne->entries.begin() + std::distance(ne->entries.begin(), i));
      } else {
        return;
      }
      if(m_data.compare_exchange_weak(old, ne)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::remove_preferred_destination(Venue venue) {
    while(true) {
      auto old = m_data.load();
      if(!old->preferred_destinations.count(venue)) {
        return;
      }
      auto ne = std::make_shared<Data>(*old);
      ne->preferred_destinations.erase(venue);
      if(m_data.compare_exchange_weak(old, ne)) {
        break;
      }
    }
  }

  inline DestinationDatabase& DestinationDatabase::operator =(
      const DestinationDatabase& other) noexcept {
    m_data.store(other.m_data.load());
    return *this;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::DestinationDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::DestinationDatabase::Entry& value, unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("venues", value.m_venues);
      shuttle.Shuttle("descriptions", value.m_description);
    }
  };

  template<>
  struct Shuttle<Nexus::DestinationDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::DestinationDatabase& value,
        unsigned int version) {
#if 0 // TODO
      shuttle.Shuttle("entries", value.m_entries);
      shuttle.Shuttle("preferred_destinations", value.m_preferred_destinations);
      shuttle.Shuttle("manual_order_entry_destination",
        value.m_manual_order_entry_destination);
#endif
    }
  };
}

#endif
