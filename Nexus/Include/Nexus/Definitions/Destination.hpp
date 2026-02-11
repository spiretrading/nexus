#ifndef NEXUS_DEFINITIONS_DESTINATION_HPP
#define NEXUS_DEFINITIONS_DESTINATION_HPP
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
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

        bool operator ==(const Entry&) const = default;
      };

      /** The entry returned for any failed lookup. */
      inline static const auto NONE = Entry();

      /** Constructs an empty DestinationDatabase. */
      DestinationDatabase() = default;

      DestinationDatabase(const DestinationDatabase& database) noexcept;

      /** Returns the list of destinations represented. */
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
      const Entry& select_first(P predicate) const;

      /**
       * Returns all Entries matching a predicate.
       * @param predicate The predicate to match against.
       */
      template<typename P>
      std::vector<Entry> select_all(P predicate) const;

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
      void set_preferred_destination(
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

      DestinationDatabase& operator =(const DestinationDatabase&) noexcept;

    private:
      friend struct Beam::Shuttle<DestinationDatabase>;
      struct Data {
        std::vector<Entry> m_entries;
        std::unordered_map<Venue, Destination> m_preferred_destinations;
        boost::optional<Entry> m_manual_order_entry_destination;
      };
      std::atomic<std::shared_ptr<Data>> m_data;
  };

  /**
   * Parses a DestinationDatabase Entry from a YAML node.
   * @param node The node to parse the DestinationDatabase Entry from.
   * @param database The VenueDatabase used to parse Venues.
   * @return The DestinationDatabase Entry represented by the <i>node</i>.
   */
  inline DestinationDatabase::Entry parse_destination_database_entry(
      const YAML::Node& node, const VenueDatabase& database) {
    return Beam::try_or_nest([&] {
      auto entry = DestinationDatabase::Entry();
      entry.m_id = Beam::extract<std::string>(node, "id");
      auto names = Beam::extract<std::vector<std::string>>(node, "venues");
      for(auto& name : names) {
        auto venue = parse_venue(name, database);
        if(!venue) {
          boost::throw_with_location(
            Beam::make_yaml_parser_exception("Invalid venue.", node.Mark()));
        }
        entry.m_venues.push_back(venue);
      }
      entry.m_description = Beam::extract<std::string>(node, "description");
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
    return Beam::try_or_nest([&] {
      auto destination_database = DestinationDatabase();
      for(auto node : Beam::get_node(node, "destinations")) {
        auto entry = parse_destination_database_entry(node, database);
        destination_database.add(entry);
      }
      for(auto node : Beam::get_node(node, "preferred_destinations")) {
        auto venue =
          parse_venue(Beam::extract<std::string>(node, "venue"), database);
        if(!venue) {
          boost::throw_with_location(
            Beam::make_yaml_parser_exception("Invalid venue.", node.Mark()));
        }
        auto destination = Beam::extract<std::string>(node, "destination");
        if(destination_database.from(destination).m_id.empty()) {
          boost::throw_with_location(Beam::make_yaml_parser_exception(
            "Invalid destination.", node.Mark()));
        }
        destination_database.set_preferred_destination(venue, destination);
      }
      auto manual_order_entry = parse_destination_database_entry(
        Beam::get_node(node, "manual_order_entry"), database);
      destination_database.set_manual_order_entry_destination(
        manual_order_entry);
      return destination_database;
    }, std::runtime_error("Failed to parse destination database."));
  }

  inline DestinationDatabase::DestinationDatabase(
    const DestinationDatabase& database) noexcept
    : m_data(database.m_data.load()) {}

  inline Beam::View<const DestinationDatabase::Entry>
      DestinationDatabase::get_entries() const {
    if(auto data = m_data.load()) {
      auto entries =
        std::shared_ptr<std::vector<Entry>>(data, &data->m_entries);
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::from(const Destination& id) const {
    if(auto data = m_data.load()) {
      auto i =
        std::lower_bound(data->m_entries.begin(), data->m_entries.end(), id,
          [] (auto const& lhs, auto const& rhs) {
            return lhs.m_id < rhs;
          });
      if(i != data->m_entries.end() && i->m_id == id) {
        return *i;
      }
    }
    return NONE;
  }

  inline const DestinationDatabase::Entry&
      DestinationDatabase::get_preferred_destination(Venue venue) const {
    if(auto data = m_data.load()) {
      auto i = data->m_preferred_destinations.find(venue);
      if(i != data->m_preferred_destinations.end()) {
        return from(i->second);
      }
    }
    return NONE;
  }

  template<typename P>
  const DestinationDatabase::Entry&
      DestinationDatabase::select_first(P predicate) const {
    if(auto data = m_data.load()) {
      for(auto& entry : data->m_entries) {
        if(predicate(entry)) {
          return entry;
        }
      }
    }
    return NONE;
  }

  template<typename P>
  std::vector<DestinationDatabase::Entry>
      DestinationDatabase::select_all(P predicate) const {
    auto result = std::vector<Entry>();
    if(auto data = m_data.load()) {
      for(auto& entry : data->m_entries) {
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
      return data->m_manual_order_entry_destination;
    }
    static const auto NONE = boost::optional<DestinationDatabase::Entry>();
    return NONE;
  }

  inline void DestinationDatabase::set_manual_order_entry_destination(
      const Entry& entry) {
    while(true) {
      auto data = m_data.load();
      auto new_data = [&] {
        if(!data) {
          auto new_data = std::make_shared<Data>();
          new_data->m_manual_order_entry_destination = entry;
          return new_data;
        }
        if(data->m_manual_order_entry_destination == entry) {
          return std::shared_ptr<Data>();
        }
        auto new_data = std::make_shared<Data>(*data);
        new_data->m_manual_order_entry_destination = entry;
        return new_data;
      }();
      if(!new_data || m_data.compare_exchange_weak(data, new_data)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::add(const Entry& entry) {
    while(true) {
      auto data = m_data.load();
      auto new_data = [&] {
        if(!data) {
          auto new_data = std::make_shared<Data>();
          new_data->m_entries.push_back(entry);
          return new_data;
        }
        auto i = std::lower_bound(
          data->m_entries.begin(), data->m_entries.end(), entry,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_id < rhs.m_id;
          });
        if(i == data->m_entries.end() || i->m_id != entry.m_id) {
          auto new_data = std::make_shared<Data>(*data);
          new_data->m_entries.insert(new_data->m_entries.begin() +
            std::distance(data->m_entries.begin(), i), entry);
          return new_data;
        }
        return std::shared_ptr<Data>();
      }();
      if(!new_data || m_data.compare_exchange_weak(data, new_data)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::set_preferred_destination(
      Venue venue, const Destination& destination) {
    while(true) {
      auto data = m_data.load();
      auto new_data = [&] {
        if(!data) {
          auto new_data = std::make_shared<Data>();
          new_data->m_preferred_destinations[venue] = destination;
          return new_data;
        }
        auto i = data->m_preferred_destinations.find(venue);
        if(i == data->m_preferred_destinations.end() ||
            i->second != destination) {
          auto new_data = std::make_shared<Data>(*data);
          new_data->m_preferred_destinations[venue] = destination;
          return new_data;
        }
        return std::shared_ptr<Data>();
      }();
      if(!new_data || m_data.compare_exchange_weak(data, new_data)) {
        break;
      }
    }
  }

  inline void DestinationDatabase::remove(const Destination& id) {
    while(true) {
      auto data = m_data.load();
      if(!data) {
        break;
      }
      auto i = std::lower_bound(
        data->m_entries.begin(), data->m_entries.end(), id,
        [] (const auto& lhs, const auto& rhs) {
          return lhs.m_id < rhs;
        });
      if(i != data->m_entries.end() && i->m_id == id) {
        auto new_data = std::make_shared<Data>(*data);
        new_data->m_entries.erase(new_data->m_entries.begin() +
          std::distance(data->m_entries.begin(), i));
        if(m_data.compare_exchange_weak(data, new_data)) {
          break;
        }
      } else {
        break;
      }
    }
  }

  inline void DestinationDatabase::remove_preferred_destination(Venue venue) {
    while(true) {
      auto data = m_data.load();
      if(!data || data->m_preferred_destinations.count(venue) == 0) {
        break;
      }
      auto new_data = std::make_shared<Data>(*data);
      new_data->m_preferred_destinations.erase(venue);
      if(m_data.compare_exchange_weak(data, new_data)) {
        break;
      }
    }
  }

  inline DestinationDatabase& DestinationDatabase::operator =(
      const DestinationDatabase& database) noexcept {
    m_data.store(database.m_data.load());
    return *this;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::DestinationDatabase::Entry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::DestinationDatabase::Entry& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("venues", value.m_venues);
      shuttle.shuttle("description", value.m_description);
    }
  };

  template<>
  struct Shuttle<Nexus::DestinationDatabase> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::DestinationDatabase& value,
        unsigned int version) const {
      if constexpr(IsSender<S>) {
        if(auto data = value.m_data.load()) {
          shuttle.send("entries", data->m_entries);
          shuttle.send(
            "preferred_destinations", data->m_preferred_destinations);
          shuttle.send("manual_order_entry_destination",
            data->m_manual_order_entry_destination);
        }
      } else {
        auto data = std::make_shared<Nexus::DestinationDatabase::Data>();
        shuttle.shuttle("entries", data->m_entries);
        shuttle.shuttle(
          "preferred_destinations", data->m_preferred_destinations);
        shuttle.shuttle("manual_order_entry_destination",
          data->m_manual_order_entry_destination);
        value.m_data.store(std::move(data));
      }
    }
  };
}

#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"

#endif
