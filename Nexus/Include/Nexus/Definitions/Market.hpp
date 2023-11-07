#ifndef NEXUS_MARKET_HPP
#define NEXUS_MARKET_HPP
#include <algorithm>
#include <ostream>
#include <string>
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

  /** Represents a market. */
  using MarketCode = Beam::FixedString<4>;

  /** Stores the database of all markets. **/
  class MarketDatabase {
    public:

      /** Stores a single entry in a MarketDatabase. **/
      struct Entry {

        /** The market identifier code. */
        MarketCode m_code;

        /** The market's country code. */
        CountryCode m_countryCode;

        /** The market's time zone. */
        std::string m_timeZone;

        /** The default currency used. */
        CurrencyId m_currency;

        /** The institution's description. */
        std::string m_description;

        /** The common display name. */
        std::string m_displayName;

        bool operator ==(const Entry& rhs) const = default;
      };

      /** Returns an Entry representing no market. */
      static const Entry& GetNoneEntry();

      /** Constructs an empty MarketDatabase. */
      MarketDatabase() = default;

      /** Returns all Entries. */
      const std::vector<Entry>& GetEntries() const;

      /**
       * Returns an Entry from its MarketCode.
       * @param code The MarketCode to lookup.
       * @return The Entry with the specified <i>code</i>.
       */
      const Entry& FromCode(MarketCode code) const;

      /**
       * Returns an Entry from its display name.
       * @param displayName The market's display name.
       * @return The Entry with the specified <i>displayName</i>.
       */
      const Entry& FromDisplayName(const std::string& displayName) const;

      /**
       * Returns all Entries participating in a specified country.
       * @param country The CountryCode to lookup.
       * @return The list of all Entries with the specified <i>country</i>.
       */
      std::vector<Entry> FromCountry(CountryCode country) const;

      /**
       * Adds an Entry.
       * @param entry The Entry to add.
       */
      void Add(const Entry& entry);

      /**
       * Deletes an Entry.
       * @param code The MarketCode of the Entry to delete.
       */
      void Delete(const MarketCode& code);

    private:
      friend struct Beam::Serialization::Shuttle<MarketDatabase>;
      static Entry MakeNoneEntry();
      std::vector<Entry> m_entries;
  };

  /**
   * Returns the time of the start of day relative to a specified market in
   * UTC.
   * @param marketCode The market whose start of day in UTC is to be returned.
   * @param dateTime The date/time, in UTC, to convert into the market's start
   *        of day, in UTC.
   * @param marketDatabase The MarketDatabase to use for time zone info.
   * @param timeZoneDatabase The time zone database to use for time zone
   *        conversions.
   * @return Takes the date represented by <i>dateTime</i>, converts it into the
   *         market's local time, truncates the time of day so that the time of
   *         day is the beginning of the day in that market's local time, then
   *         converts that value back to UTC.
   */
  inline boost::posix_time::ptime MarketDateToUtc(MarketCode marketCode,
      const boost::posix_time::ptime& dateTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto marketTimeZone = timeZoneDatabase.time_zone_from_region(
      marketDatabase.FromCode(marketCode).m_timeZone);
    auto utcTimeZone = timeZoneDatabase.time_zone_from_region("UTC");
    auto universalDateTime = boost::local_time::local_date_time(dateTime.date(),
      dateTime.time_of_day(), utcTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    auto currentMarketDateTime = universalDateTime.local_time_in(
      marketTimeZone);
    auto marketCutOffDateTime = boost::posix_time::ptime(
      currentMarketDateTime.local_time().date(), boost::posix_time::seconds(0));
    auto marketCutOffLocalDateTime = boost::local_time::local_date_time(
      marketCutOffDateTime.date(), marketCutOffDateTime.time_of_day(),
      marketTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    auto universalCutOffTime = marketCutOffLocalDateTime.local_time_in(
      utcTimeZone);
    auto utcMarketDate = boost::posix_time::ptime(
      universalCutOffTime.local_time().date(),
      universalCutOffTime.local_time().time_of_day());
    return utcMarketDate;
  }

  /**
   * Parses a MarketCode from a string.
   * @param source The string to parse.
   * @param database The MarketDatabase containing the available MarketCodes.
   * @return The MarketCode represented by the <i>source</i>.
   */
  inline MarketCode ParseMarketCode(const std::string& source,
      const MarketDatabase& database) {
    auto& entry = database.FromDisplayName(source);
    if(entry.m_code == MarketCode()) {
      return database.FromCode(source).m_code;
    }
    return entry.m_code;
  }

  /**
   * Parses a MarketEntry from a string.
   * @param source The string to parse.
   * @param database The MarketDatabase containing the available MarketEntry.
   * @return The MarketCode represented by the <i>source</i>.
   */
  inline const MarketDatabase::Entry& ParseMarketEntry(
      const std::string& source, const MarketDatabase& database) {
    auto& entry = database.FromDisplayName(source);
    if(entry.m_code == MarketCode()) {
      return database.FromCode(source);
    }
    return entry;
  }

  /**
   * Parses a MarketDatabase Entry from a YAML node.
   * @param node The node to parse the MarketDatabase Entry from.
   * @return The MarketDatabase Entry represented by the <i>node</i>.
   */
  inline MarketDatabase::Entry ParseMarketDatabaseEntry(const YAML::Node& node,
      const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    return Beam::TryOrNest([&] {
      auto entry = MarketDatabase::Entry();
      entry.m_code = Beam::Extract<std::string>(node, "code");
      entry.m_countryCode = ParseCountryCode(
        Beam::Extract<std::string>(node, "country_code"), countryDatabase);
      if(entry.m_countryCode == CountryCode::NONE) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
          "Invalid country code.", node.Mark()));
      }
      entry.m_timeZone = Beam::Extract<std::string>(node, "time_zone");
      entry.m_currency = ParseCurrency(
        Beam::Extract<std::string>(node, "currency"), currencyDatabase);
      if(entry.m_currency == CurrencyId::NONE) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid currency.",
          node.Mark()));
      }
      entry.m_description = Beam::Extract<std::string>(node, "description");
      entry.m_displayName = Beam::Extract<std::string>(node, "display_name");
      return entry;
    }, std::runtime_error("Failed to parse market database entry."));
  }

  /**
   * Parses a MarketDatabase from a YAML node.
   * @param node The node to parse the MarketDatabase from.
   * @param countryDatabase The CountryDatabase used to parse country codes.
   * @param currencyDatabase The CurrencyDatabase used to parse currencies.
   * @return The MarketDatabase represented by the <i>node</i>.
   */
  inline MarketDatabase ParseMarketDatabase(const YAML::Node& node,
      const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    return Beam::TryOrNest([&] {
      auto marketDatabase = MarketDatabase();
      for(auto& node : node) {
        marketDatabase.Add(ParseMarketDatabaseEntry(node, countryDatabase,
          currencyDatabase));
      }
      return marketDatabase;
    }, std::runtime_error("Failed to parse market database."));
  }

  inline std::string ToString(
      MarketCode value, const MarketDatabase& marketDatabase) {
    auto& market = marketDatabase.FromCode(value);
    if(market.m_code.IsEmpty()) {
      return value.GetData();
    } else {
      return market.m_displayName;
    }
  }

  inline const MarketDatabase::Entry& MarketDatabase::GetNoneEntry() {
    static auto NONE = MakeNoneEntry();
    return NONE;
  }

  inline const std::vector<MarketDatabase::Entry>&
      MarketDatabase::GetEntries() const {
    return m_entries;
  }

  inline const MarketDatabase::Entry& MarketDatabase::FromCode(
      MarketCode code) const {
    auto comparator = Entry();
    comparator.m_code = code;
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(),
      comparator,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(i != m_entries.end() && i->m_code == code) {
      return *i;
    }
    return GetNoneEntry();
  }

  inline const MarketDatabase::Entry& MarketDatabase::FromDisplayName(
      const std::string& displayName) const {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_displayName == displayName;
      });
    if(i == m_entries.end()) {
      return GetNoneEntry();
    }
    return *i;
  }

  inline std::vector<MarketDatabase::Entry> MarketDatabase::FromCountry(
      CountryCode country) const {
    auto entries = std::vector<MarketDatabase::Entry>();
    std::copy_if(m_entries.begin(), m_entries.end(),
      std::back_inserter(entries),
      [&] (auto& entry) {
        return entry.m_countryCode == country;
      });
    return entries;
  }

  inline void MarketDatabase::Add(const Entry& entry) {
    auto i = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(i == m_entries.end() || i->m_code != entry.m_code) {
      m_entries.insert(i, entry);
    }
  }

  inline void MarketDatabase::Delete(const MarketCode& code) {
    auto i = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (auto& entry) {
        return entry.m_code == code;
      });
    if(i == m_entries.end()) {
      return;
    }
    m_entries.erase(i);
  }

  inline MarketDatabase::Entry MarketDatabase::MakeNoneEntry() {
    auto entry = Entry();
    entry.m_code = MarketCode();
    entry.m_countryCode = CountryCode::NONE;
    entry.m_timeZone = "UTC";
    entry.m_description = "None";
    return entry;
  }

  inline std::ostream& operator <<(std::ostream& out,
      const MarketDatabase::Entry& entry) {
    return out << '(' << entry.m_code << ' ' << entry.m_countryCode << ' ' <<
      entry.m_timeZone << ' ' << entry.m_currency << ' ' <<
      entry.m_description << ' ' << entry.m_displayName << ')';
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::MarketDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("country_code", value.m_countryCode);
      shuttle.Shuttle("time_zone", value.m_timeZone);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("display_name", value.m_displayName);
    }
  };

  template<>
  struct Shuttle<Nexus::MarketDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::MarketDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
    }
  };
}

#endif
