#ifndef NEXUS_VENUE_HPP
#define NEXUS_VENUE_HPP
#include <algorithm>
#include <atomic>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/ScopedStreamManipulator.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Represents a trading venue, identified by a market identifier code. */
  class Venue {
    public:

      /** The type used to store the MIC. */
      using Code = Beam::FixedString<4>;

      /** Constructs an empty Venue. */
      Venue() = default;

      /**
       * @brief Constructs a Venue from a market identifier code.
       * @param mic The market identifier code.
       */
      explicit Venue(Code mic) noexcept;

      /** Returns the MIC. */
      Code get_code() const;

      /** Returns true if this is not an empty venue. */
      explicit operator bool() const;

      auto operator <=>(const Venue&) const = default;

    private:
      Beam::FixedString<4> m_mic;
  };

  /** Stores a database of trading venues. */
  class VenueDatabase {
    public:

      /** Stores a single entry in a VenueDatabase. */
      struct Entry {

        /** The venue. */
        Venue m_venue;

        /** The venue's country code. */
        CountryCode m_country_code;

        /** The default market center used for trades on the venue. */
        std::string m_market_center;

        /** The venue's time zone. */
        std::string m_time_zone;

        /** The default currency used. */
        CurrencyId m_currency;

        /** The venue's description. */
        std::string m_description;

        /** The common display name. */
        std::string m_display_name;

        bool operator ==(const Entry&) const = default;
      };

      /** An Entry representing no venue. */
      inline static const auto NONE = Entry();

      /** Constructs an empty VenueDatabase. */
      VenueDatabase() = default;

      VenueDatabase(const VenueDatabase& database) noexcept;

      /** Returns all Entries. */
      Beam::View<const Entry> get_entries() const;

      /**
       * Returns an Entry from its Venue.
       * @param venue The Venue to lookup.
       * @return The Entry with the specified venue.
       */
      const Entry& from(Venue venue) const;

      /**
       * Returns an Entry from its Venue.
       * @param venue The Venue to lookup.
       * @return The Entry with the specified venue.
       */
      const Entry& from(std::string_view venue) const;

      /**
       * Returns an Entry from its display name.
       * @param display_name The venue's display name.
       * @return The Entry with the specified display_name.
       */
      const Entry& from_display_name(std::string_view display_name) const;

      /**
       * Returns all Entries participating in a specified country.
       * @param country The CountryCode to lookup.
       * @return The list of all Entries with the specified country.
       */
      std::vector<Entry> from(CountryCode country) const;

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

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Removes an Entry.
       * @param venue The Venue to delete.
       */
      void remove(Venue venue);

      VenueDatabase& operator =(const VenueDatabase&) noexcept;

    private:
      friend struct Beam::Serialization::Shuttle<VenueDatabase>;
      std::atomic<std::shared_ptr<std::vector<Entry>>> m_entries;
  };

  /**
   * Parses an Entry from a string.
   * @param source The string to parse.
   * @param database The VenueDatabase containing the available Entry.
   * @return The Entry represented by the <i>source</i>.
   */
  inline const VenueDatabase::Entry& parse_venue_entry(
      std::string_view source, const VenueDatabase& database) {
    auto& entry = database.from_display_name(source);
    if(!entry.m_venue) {
      return database.from(Venue(source));
    }
    return entry;
  }

  /**
   * Parses an Entry from a string using the default venue database.
   * @param source The string to parse.
   * @return The Entry represented by the <i>source</i>.
   */
  inline const VenueDatabase::Entry& parse_venue_entry(
      std::string_view source) {
    extern const VenueDatabase& DEFAULT_VENUES;
    return parse_venue_entry(source, DEFAULT_VENUES);
  }

  /**
   * Parses a Venue from a string.
   * @param source The string to parse.
   * @param database The VenueDatabase containing the available Venues to parse.
   * @return The Venue represented by the <i>source</i>.
   */
  inline Venue parse_venue(
      std::string_view source, const VenueDatabase& database) {
    return parse_venue_entry(source, database).m_venue;
  }

  /**
   * Parses a Venue from a string using the default database.
   * @param source The string to parse.
   * @return The Venue represented by the <i>source</i>.
   */
  inline Venue parse_venue(std::string_view source) {
    extern const VenueDatabase& DEFAULT_VENUES;
    return parse_venue(source, DEFAULT_VENUES);
  }

  /**
   * Parses an Entry from a YAML node.
   * @param node The node to parse the VenueDatabase Entry from.
   * @return The Entry represented by the <i>node</i>.
   */
  inline VenueDatabase::Entry parse_venue_database_entry(
      const YAML::Node& node, const CountryDatabase& country_database,
      const CurrencyDatabase& currency_database) {
    return Beam::TryOrNest([&] {
      auto entry = VenueDatabase::Entry();
      entry.m_venue = Venue(Beam::Extract<std::string>(node, "venue"));
      entry.m_country_code = parse_country_code(
        Beam::Extract<std::string>(node, "country_code"), country_database);
      if(!entry.m_country_code) {
        BOOST_THROW_EXCEPTION(
          Beam::MakeYamlParserException("Invalid country code.", node.Mark()));
      }
      entry.m_time_zone = Beam::Extract<std::string>(node, "time_zone");
      entry.m_currency = parse_currency(
        Beam::Extract<std::string>(node, "currency"), currency_database);
      if(!entry.m_currency) {
        BOOST_THROW_EXCEPTION(
          Beam::MakeYamlParserException("Invalid currency.", node.Mark()));
      }
      entry.m_description = Beam::Extract<std::string>(node, "description");
      entry.m_display_name = Beam::Extract<std::string>(node, "display_name");
      entry.m_market_center = Beam::Extract<std::string>(node, "market_center",
        entry.m_display_name);
      return entry;
    }, std::runtime_error("Failed to parse venue database entry."));
  }

  /**
   * Parses a VenueDatabase from a YAML node.
   * @param node The node to parse the VenueDatabase from.
   * @param country_database The CountryDatabase used to parse country codes.
   * @param currency_database The CurrencyDatabase used to parse currencies.
   * @return The VenueDatabase represented by the <i>node</i>.
   */
  inline VenueDatabase parse_venue_database(
      const YAML::Node& node, const CountryDatabase& country_database,
      const CurrencyDatabase& currency_database) {
    return Beam::TryOrNest([&] {
      auto database = VenueDatabase();
      for(auto& node : node) {
        database.add(parse_venue_database_entry(
          node, country_database, currency_database));
      }
      return database;
    }, std::runtime_error("Failed to parse venue database."));
  }

  inline auto operator <<(std::ostream& out, const VenueDatabase& database) {
    return Beam::ScopedStreamManipulator(out, database);
  }

  inline std::ostream& operator <<(std::ostream& out, Venue venue) {
    extern const VenueDatabase& DEFAULT_VENUES;
    auto database = static_cast<const VenueDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID));
    if(!database) {
      database = &DEFAULT_VENUES;
    }
    auto& entry = database->from(venue);
    if(entry.m_venue) {
      return out << entry.m_display_name;
    }
    return out << venue.get_code();
  }

  inline std::ostream& operator <<(std::ostream& out,
      const VenueDatabase::Entry& entry) {
    return out << '(' << entry.m_venue << ' ' << entry.m_country_code << ' ' <<
      entry.m_market_center << ' ' << entry.m_time_zone << ' ' <<
      entry.m_currency << ' ' << entry.m_description << ' ' <<
      entry.m_display_name << ')';
  }

  /**
   * Returns the time of the start of day relative to a specified venue in UTC.
   * @param venue The venue whose start of day in UTC is to be returned.
   * @param date_time The date/time, in UTC, to convert into the venue's start
   *        of day, in UTC.
   * @param venue_database The VenueDatabase to use for time zone info.
   * @param time_zone_database The time zone database to use for time zone
   *        conversions.
   * @return Takes the date represented by <i>date_time</i>, converts it into
   *         the venue's local time, truncates the time of day so that the time
   *         of day is the beginning of the day in that venue's local time, then
   *         converts that value back to UTC.
   */
  inline boost::posix_time::ptime venue_date_to_utc(Venue venue,
      boost::posix_time::ptime date_time, const VenueDatabase& venue_database,
      const boost::local_time::tz_database& time_zone_database) {
    auto venue_time_zone = time_zone_database.time_zone_from_region(
      venue_database.from(venue).m_time_zone);
    auto venue_local_date = boost::local_time::local_date_time(
      date_time.date(), boost::posix_time::seconds(0), venue_time_zone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    return venue_local_date.utc_time();
  }

  inline Venue::Venue(Code mic) noexcept
    : m_mic(std::move(mic)) {}

  inline Venue::Code Venue::get_code() const {
    return m_mic;
  }

  inline Venue::operator bool() const {
    return !m_mic.IsEmpty();
  }

  inline VenueDatabase::VenueDatabase(const VenueDatabase& database) noexcept
    : m_entries(database.m_entries.load()) {}

  inline Beam::View<const VenueDatabase::Entry>
      VenueDatabase::get_entries() const {
    if(auto entries = m_entries.load()) {
      return Beam::View(Beam::SharedIterator(entries, entries->begin()),
        Beam::SharedIterator(entries, entries->end()));
    }
    return Beam::View<const Entry>();
  }

  inline const VenueDatabase::Entry& VenueDatabase::from(Venue venue) const {
    if(auto entries = m_entries.load()) {
      auto i = std::lower_bound(entries->begin(), entries->end(), venue,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_venue < rhs;
        });
      if(i != entries->end() && i->m_venue == venue) {
        return *i;
      }
    }
    return NONE;
  }

  inline const VenueDatabase::Entry&
      VenueDatabase::from(std::string_view venue) const {
    return from(Venue(venue));
  }

  inline const VenueDatabase::Entry&
      VenueDatabase::from_display_name(std::string_view display_name) const {
    if(auto entries = m_entries.load()) {
      auto i = std::find_if(entries->begin(), entries->end(),
        [&] (const auto& entry) {
          return entry.m_display_name == display_name;
        });
      if(i != entries->end()) {
        return *i;
      }
    }
    return NONE;
  }

  inline std::vector<VenueDatabase::Entry>
      VenueDatabase::from(CountryCode country) const {
    auto result = std::vector<VenueDatabase::Entry>();
    if(auto entries = m_entries.load()) {
      std::copy_if(entries->begin(), entries->end(), std::back_inserter(result),
        [&] (const auto& entry) {
          return entry.m_country_code == country;
        });
    }
    return result;
  }

  template<typename P>
  const VenueDatabase::Entry& VenueDatabase::select_first(P predicate) const {
    if(auto entries = m_entries.load()) {
      for(auto& entry : *entries) {
        if(predicate(entry)) {
          return entry;
        }
      }
    }
    return NONE;
  }

  template<typename P>
  std::vector<VenueDatabase::Entry>
      VenueDatabase::select_all(P predicate) const {
    auto result = std::vector<Entry>();
    if(auto entries = m_entries.load()) {
      for(auto& entry : *entries) {
        if(predicate(entry)) {
          result.push_back(entry);
        }
      }
    }
    return result;
  }

  inline void VenueDatabase::add(const Entry& entry) {
    while(true) {
      auto entries = m_entries.load();
      auto new_entries = [&] {
        if(!entries) {
          auto new_entries = std::make_shared<std::vector<Entry>>();
          new_entries->push_back(entry);
          return new_entries;
        }
        auto i = std::lower_bound(entries->begin(), entries->end(), entry,
          [] (const auto& lhs, const auto& rhs) {
            return lhs.m_venue < rhs.m_venue;
          });
        if(i == entries->end() || i->m_venue != entry.m_venue) {
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

  inline void VenueDatabase::remove(Venue venue) {
    while(true) {
      auto entries = m_entries.load();
      if(!entries) {
        break;
      }
      auto i = std::lower_bound(entries->begin(), entries->end(), venue,
        [] (const auto& lhs, auto rhs) {
          return lhs.m_venue < rhs;
        });
      if(i != entries->end() && i->m_venue == venue) {
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

  inline VenueDatabase& VenueDatabase::operator =(
      const VenueDatabase& database) noexcept {
    m_entries.store(database.m_entries.load());
    return *this;
  }

  inline std::size_t hash_value(Venue venue) {
    return std::hash<Nexus::Venue::Code>()(venue.get_code());
  }
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<Nexus::Venue> : std::false_type {};

  template<>
  struct Send<Nexus::Venue> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, Nexus::Venue value) const {
      shuttle.Send(name, value.get_code());
    }
  };

  template<>
  struct Receive<Nexus::Venue> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, Nexus::Venue& value) const {
      auto code = Nexus::Venue::Code();
      shuttle.Shuttle(name, code);
      value = Nexus::Venue(code);
    }
  };

  template<>
  struct Shuttle<Nexus::VenueDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::VenueDatabase::Entry& value,
        unsigned int version) const {
      shuttle.Shuttle("venue", value.m_venue);
      shuttle.Shuttle("country_code", value.m_country_code);
      shuttle.Shuttle("market_center", value.m_market_center);
      shuttle.Shuttle("time_zone", value.m_time_zone);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("display_name", value.m_display_name);
    }
  };

  template<>
  struct Shuttle<Nexus::VenueDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::VenueDatabase& value,
        unsigned int version) const {
      if constexpr(IsSender<Shuttler>::value) {
        if(auto entries = value.m_entries.load()) {
          shuttle.Send("entries", *entries);
        }
      } else {
        auto entries =
          std::make_shared<std::vector<Nexus::VenueDatabase::Entry>>();
        shuttle.Shuttle("entries", *entries);
        value.m_entries.store(std::move(entries));
      }
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Venue> {
    std::size_t operator ()(Nexus::Venue venue) const {
      return hash_value(venue);
    }
  };
}

#include "Nexus/Definitions/DefaultVenueDatabase.hpp"

#endif
