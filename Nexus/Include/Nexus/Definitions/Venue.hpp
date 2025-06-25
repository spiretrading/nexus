#ifndef NEXUS_VENUE_HPP
#define NEXUS_VENUE_HPP
#include <algorithm>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Represents a trading venue, identified by a market identifier code. */
  class Venue {
    public:

      /** Constructs an empty Venue. */
      Venue() = default;

      /**
       * @brief Constructs a Venue from a market identifier code.
       * @param mic The market identifier code.
       */
      Venue(Beam::FixedString<4> mic);

      auto operator <=>(const Venue&) const = default;

    private:
      Beam::FixedString<4> m_mic;
  };

  /** Stores a database of trading venues. **/
  class VenueDatabase {
    public:

      /** Stores a single entry in a VenueDatabase. */
      struct Entry {

        /** The venue. */
        Venue m_venue;

        /** The venue's country code. */
        CountryCode m_country_code;

        /** The venue's time zone. */
        std::string m_time_zone;

        /** The default currency used. */
        CurrencyId m_currency;

        /** The venue's description. */
        std::string m_description;

        /** The common display name. */
        std::string m_display_name;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** An Entry representing no venue. */
      inline static const auto NONE = Entry();

      /** Constructs an empty VenueDatabase. */
      VenueDatabase() = default;

      /** Returns all Entries. */
      const std::vector<Entry>& get_entries() const;

      /**
       * Returns an Entry from its Venue.
       * @param venue The Venue to lookup.
       * @return The Entry with the specified venue.
       */
      const Entry& from(Venue venue) const;

      /**
       * Returns an Entry from its display name.
       * @param display_name The market's display name.
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
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void add(const Entry& entry);

      /**
       * Removes an Entry.
       * @param venue The Venue to delete.
       */
      void remove(Venue venue);

    private:
      friend struct Beam::Serialization::Shuttle<VenueDatabase>;
      std::vector<Entry> m_entries;
  };

#if 0
  /**
   * Returns the time of the start of day relative to a specified venue in UTC.
   * @param venue The venue whose start of day in UTC is to be returned.
   * @param dateTime The date/time, in UTC, to convert into the venue's start of
   *        day, in UTC.
   * @param venueDatabase The VenueDatabase to use for time zone info.
   * @param timeZoneDatabase The time zone database to use for time zone
   *        conversions.
   * @return Takes the date represented by <i>dateTime</i>, converts it into the
   *         venue's local time, truncates the time of day so that the time of
   *         day is the beginning of the day in that venue's local time, then
   *         converts that value back to UTC.
   */
  inline boost::posix_time::ptime VenueDateToUtc(Venue venue,
      boost::posix_time::ptime dateTime, const VenueDatabase& venueDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto venueTimeZone = timeZoneDatabase.time_zone_from_region(
      venueDatabase.From(venue).m_timeZone);
    auto utcTimeZone = timeZoneDatabase.time_zone_from_region("UTC");
    auto universalDateTime = boost::local_time::local_date_time(dateTime.date(),
      dateTime.time_of_day(), utcTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    auto currentVenueDateTime = universalDateTime.local_time_in(venueTimeZone);
    auto venueCutOffDateTime = boost::posix_time::ptime(
      currentVenueDateTime.local_time().date(), boost::posix_time::seconds(0));
    auto venueCutOffLocalDateTime = boost::local_time::local_date_time(
      venueCutOffDateTime.date(), venueCutOffDateTime.time_of_day(),
      venueTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    auto universalCutOffTime = venueCutOffLocalDateTime.local_time_in(
      utcTimeZone);
    auto utcVenueDate = boost::posix_time::ptime(
      universalCutOffTime.local_time().date(),
      universalCutOffTime.local_time().time_of_day());
    return utcVenueDate;
  }

  /**
   * Parses a Venue from a string.
   * @param source The string to parse.
   * @param database The VenueDatabase containing the available Venues to parse.
   * @return The Venue represented by the <i>source</i>.
   */
  inline Venue ParseVenue(
      std::string_view source, const VenueDatabase& database) {
    auto& entry = database.FromDisplayName(source);
    if(entry.m_venue == Venue()) {
      return database.From(source).m_venue;
    }
    return entry.m_venue;
  }

  /**
   * Parses an Entry from a string.
   * @param source The string to parse.
   * @param database The VenueDatabase containing the available Entry.
   * @return The Entry represented by the <i>source</i>.
   */
  inline const VenueDatabase::Entry& ParseVenueEntry(
      std::string_view source, const VenueDatabase& database) {
    auto& entry = database.FromDisplayName(source);
    if(entry.m_venue == Venue()) {
      return database.From(source);
    }
    return entry;
  }

  /**
   * Parses an Entry from a YAML node.
   * @param node The node to parse the VenueDatabase Entry from.
   * @return The Entry represented by the <i>node</i>.
   */
  inline VenueDatabase::Entry ParseVenueDatabaseEntry(
      const YAML::Node& node, const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    return Beam::TryOrNest([&] {
      auto entry = VenueDatabase::Entry();
      entry.m_venue = Beam::Extract<std::string>(node, "code");
      entry.m_countryCode = ParseCountryCode(
        Beam::Extract<std::string>(node, "country_code"), countryDatabase);
      if(entry.m_countryCode == CountryCode::NONE) {
        BOOST_THROW_EXCEPTION(
          Beam::MakeYamlParserException("Invalid country code.", node.Mark()));
      }
      entry.m_timeZone = Beam::Extract<std::string>(node, "time_zone");
      entry.m_currency = ParseCurrency(
        Beam::Extract<std::string>(node, "currency"), currencyDatabase);
      if(entry.m_currency == CurrencyId::NONE) {
        BOOST_THROW_EXCEPTION(
          Beam::MakeYamlParserException("Invalid currency.", node.Mark()));
      }
      entry.m_description = Beam::Extract<std::string>(node, "description");
      entry.m_displayName = Beam::Extract<std::string>(node, "display_name");
      return entry;
    }, std::runtime_error("Failed to parse market database entry."));
  }

  /**
   * Parses a VenueDatabase from a YAML node.
   * @param node The node to parse the VenueDatabase from.
   * @param countryDatabase The CountryDatabase used to parse country codes.
   * @param currencyDatabase The CurrencyDatabase used to parse currencies.
   * @return The VenueDatabase represented by the <i>node</i>.
   */
  inline VenueDatabase ParseVenueDatabase(
      const YAML::Node& node, const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    return Beam::TryOrNest([&] {
      auto database = VenueDatabase();
      for(auto& node : node) {
        database.Add(
          ParseVenueDatabaseEntry(node, countryDatabase, currencyDatabase));
      }
      return database;
    }, std::runtime_error("Failed to parse venue database."));
  }

  inline std::string ToString(Venue value, const VenueDatabase& database) {
    auto& entry = database.From(value);
    if(entry.m_venue == Venue()) {
      return value.GetData();
    } else {
      return entry.m_displayName;
    }
  }

  inline std::ostream& operator <<(std::ostream& out,
      const VenueDatabase::Entry& entry) {
    return out << '(' << entry.m_venue << ' ' << entry.m_country_code << ' ' <<
      entry.m_time_zone << ' ' << entry.m_currency << ' ' <<
      entry.m_description << ' ' << entry.m_display_name << ')';
  }
#endif

  inline Venue::Venue(Beam::FixedString<4> mic)
    : m_mic(std::move(mic)) {}

  inline const std::vector<VenueDatabase::Entry>&
      VenueDatabase::get_entries() const {
    return m_entries;
  }

  inline const VenueDatabase::Entry& VenueDatabase::from(Venue venue) const {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), venue,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_venue < rhs;
      });
    if(i != m_entries.end() && i->m_venue == venue) {
      return *i;
    }
    return NONE;
  }

  inline const VenueDatabase::Entry&
      VenueDatabase::from_display_name(std::string_view display_name) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_display_name == display_name;
      });
    if(i == m_entries.end()) {
      return NONE;
    }
    return *i;
  }

  inline std::vector<VenueDatabase::Entry>
      VenueDatabase::from(CountryCode country) const {
    auto entries = std::vector<VenueDatabase::Entry>();
    std::copy_if(m_entries.begin(), m_entries.end(),
      std::back_inserter(entries), [&] (const auto& entry) {
        return entry.m_country_code == country;
      });
    return entries;
  }

  inline void VenueDatabase::add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_venue < rhs.m_venue;
      });
    if(i == m_entries.end() || i->m_venue != entry.m_venue) {
      m_entries.insert(i, entry);
    }
  }

  inline void VenueDatabase::remove(Venue venue) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(), venue,
      [] (const auto& lhs, auto rhs) {
        return lhs.m_venue < rhs;
      });
    if(i != m_entries.end() && i->m_venue == venue) {
      m_entries.erase(i);
    }
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::VenueDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::VenueDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("venue", value.m_venue);
      shuttle.Shuttle("country_code", value.m_country_code);
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
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
    }
  };
}

#endif
