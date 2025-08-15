#ifndef NEXUS_SECURITY_HPP
#define NEXUS_SECURITY_HPP
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Identifies a tradeable instrument. */
  class Security {
    public:

      /** Constructs an empty Security. */
      Security() = default;

      /**
       * Constructs a Security.
       * @param symbol The ticker symbol.
       * @param venue The primary venue the symbol is listed on.
       */
      Security(std::string symbol, Venue venue) noexcept;

      /** Returns the symbol. */
      const std::string& get_symbol() const;

      /** Returns the venue. */
      Venue get_venue() const;

      auto operator <=>(const Security&) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<Security>;
      std::string m_symbol;
      Venue m_venue;
  };

  /**
   * Parses a Security.
   * @param source The string to parse.
   * @param database The database containing all venues.
   * @return The Security represented by the <i>source</i>.
   */
  inline Security parse_security(
      std::string_view source, const VenueDatabase& database) {
    auto separator = source.find_last_of('.');
    if(separator == std::string_view::npos || separator == 0) {
      return Security();
    }
    auto symbol = source.substr(0, separator);
    auto is_alphanumeric =
      std::none_of(symbol.begin(), symbol.end(), [] (auto c) {
        return std::isspace(static_cast<unsigned char>(c));
      });
    if(!is_alphanumeric) {
      return Security();
    }
    auto venue = parse_venue(source.substr(separator + 1), database);
    if(venue == Venue()) {
      return Security();
    }
    return Security(std::string(symbol), venue);
  }

  /**
   * Parses a Security using the default VenueDatabase.
   * @param source The string to parse.
   * @return The Security represented by the <i>source</i>.
   */
  inline Security parse_security(std::string_view source) {
    return parse_security(source, DEFAULT_VENUES);
  }

  inline std::ostream& operator <<(std::ostream& out, const Security& value) {
    if(value.get_venue() == Venue() || value.get_symbol().empty()) {
      return out << value.get_symbol();
    }
    return out << value.get_symbol() << '.' << value.get_venue();
  }

  inline std::istream& operator >>(std::istream& in, Security& value) {
    auto s = std::string();
    in >> s;
    value = parse_security(s);
    return in;
  }

  inline std::size_t hash_value(const Security& security) {
    auto seed = std::size_t(0);
    boost::hash_combine(seed, security.get_symbol());
    boost::hash_combine(seed, security.get_venue());
    return seed;
  }

  inline Security::Security(std::string symbol, Venue venue) noexcept
    : m_symbol(std::move(symbol)),
      m_venue(venue) {}

  inline const std::string& Security::get_symbol() const {
    return m_symbol;
  }

  inline Venue Security::get_venue() const {
    return m_venue;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Security> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, Nexus::Security& value, unsigned int version) const {
      shuttle.Shuttle("symbol", value.m_symbol);
      shuttle.Shuttle("venue", value.m_venue);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::Security> {
    size_t operator()(const Nexus::Security& value) const {
      return Nexus::hash_value(value);
    }
  };
}

#endif
