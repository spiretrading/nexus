#ifndef NEXUS_DEFINITIONS_TICKER_HPP
#define NEXUS_DEFINITIONS_TICKER_HPP
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Identifies a tradeable instrument. */
  class Ticker {
    public:

      /** Constructs an empty Ticker. */
      Ticker() = default;

      /**
       * Constructs a Ticker.
       * @param symbol The ticker symbol.
       */
      explicit Ticker(std::string symbol) noexcept;

      /**
       * Constructs a Ticker.
       * @param symbol The ticker symbol.
       * @param venue The primary venue the symbol is listed on.
       */
      Ticker(std::string symbol, Venue venue) noexcept;

      /** Returns the symbol. */
      const std::string& get_symbol() const;

      /** Returns the venue. */
      Venue get_venue() const;

      /** Returns true if this Ticker is not empty. */
      explicit operator bool() const;

      auto operator <=>(const Ticker&) const = default;

    private:
      friend struct Beam::Shuttle<Ticker>;
      std::string m_symbol;
      Venue m_venue;
  };

  /**
   * Parses a Ticker.
   * @param source The string to parse.
   * @param database The database containing all venues.
   * @return The Ticker represented by the <i>source</i>.
   */
  inline Ticker parse_ticker(
      std::string_view source, const VenueDatabase& database) {
    auto separator = source.find_last_of('.');
    if(separator == std::string_view::npos || separator == 0) {
      return Ticker();
    }
    auto symbol = source.substr(0, separator);
    auto is_alphanumeric =
      std::none_of(symbol.begin(), symbol.end(), [] (auto c) {
        return std::isspace(static_cast<unsigned char>(c));
      });
    if(!is_alphanumeric) {
      return Ticker();
    }
    if(auto venue = parse_venue(source.substr(separator + 1), database)) {
      return Ticker(std::string(symbol), venue);
    }
    return Ticker();
  }

  /**
   * Parses a Ticker using the default VenueDatabase.
   * @param source The string to parse.
   * @return The Ticker represented by the <i>source</i>.
   */
  inline Ticker parse_ticker(std::string_view source) {
    return parse_ticker(source, DEFAULT_VENUES);
  }

  inline std::ostream& operator <<(std::ostream& out, const Ticker& value) {
    if(!value.get_venue() || value.get_symbol().empty()) {
      return out << value.get_symbol();
    }
    return out << value.get_symbol() << '.' << value.get_venue();
  }

  inline std::istream& operator >>(std::istream& in, Ticker& value) {
    auto s = std::string();
    in >> s;
    value = parse_ticker(s);
    return in;
  }

  inline Ticker::Ticker(std::string symbol) noexcept
    : m_symbol(std::move(symbol)) {}

  inline Ticker::Ticker(std::string symbol, Venue venue) noexcept
    : m_symbol(std::move(symbol)),
      m_venue(venue) {}

  inline const std::string& Ticker::get_symbol() const {
    return m_symbol;
  }

  inline Venue Ticker::get_venue() const {
    return m_venue;
  }

  inline Ticker::operator bool() const {
    return !m_symbol.empty() && m_venue;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Ticker> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Ticker& value, unsigned int version) const {
      shuttle.shuttle("symbol", value.m_symbol);
      shuttle.shuttle("venue", value.m_venue);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::Ticker> {
    size_t operator()(const Nexus::Ticker& value) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, value.get_symbol());
      boost::hash_combine(seed, std::hash<Nexus::Venue>()(value.get_venue()));
      return seed;
    }
  };
}

namespace Nexus {

  /**
   * Parses a set of symbols from a YAML config.
   * @param config The config to parse.
   * @param venues The available venues to parse.
   * @return The set of parsed symbols.
   */
  inline std::unordered_set<Ticker> parse_ticker_set(
      const YAML::Node& config, const VenueDatabase& venues) {
    auto tickers = std::unordered_set<Ticker>();
    for(auto& item : config) {
      if(auto ticker = parse_ticker(item.as<std::string>(), venues)) {
        tickers.insert(ticker);
      }
    }
    return tickers;
  }

  /**
   * Parses a set of symbols from a YAML config using the default venues.
   * @param config The config to parse.
   * @return The set of parsed symbols.
   */
  inline std::unordered_set<Ticker> parse_ticker_set(
      const YAML::Node& config) {
    return parse_ticker_set(config, DEFAULT_VENUES);
  }
}

#endif
