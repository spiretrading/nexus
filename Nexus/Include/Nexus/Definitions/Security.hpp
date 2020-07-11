#ifndef NEXUS_SECURITY_HPP
#define NEXUS_SECURITY_HPP
#include <istream>
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  /*! \class Security
      \brief Identifies a tradeable instrument.
   */
  class Security {
    public:

      //! Constructs an uninitialized Security.
      Security();

      //! Constructs a Security.
      /*!
        \param symbol The ticker symbol.
        \param market The primary market the symbol is listed on.
        \param country The country the symbol is listed on.
      */
      Security(std::string symbol, MarketCode market, CountryCode country);

      //! Constructs a Security.
      /*!
        \param symbol The ticker symbol.
        \param country The country the symbol is listed on.
      */
      Security(std::string symbol, CountryCode country);

      //! Tests whether this Security is less than another.
      /*!
        \param rhs The right hand side of the comparison.
        \return <code>true</code> iff <i>this</i> has lesser ordering than
                <i>rhs</i>.
      */
      bool operator <(const Security& rhs) const;

      //! Tests whether this Security is equal to another.
      /*!
        \param rhs The right hand side of the equality.
        \return <code>true</code> iff <i>this</i> has the same key as
                <i>rhs</i>.
      */
      bool operator ==(const Security& rhs) const;

      //! Tests whether this Security is not equal to another.
      /*!
        \param rhs The right hand side of the equality.
        \return <code>true</code> iff <i>this</i> has a different key as
                <i>rhs</i>.
      */
      bool operator !=(const Security& rhs) const;

      //! Returns the symbol.
      const std::string& GetSymbol() const;

      //! Returns the Market.
      MarketCode GetMarket() const;

      //! Returns the Country that issued the security.
      CountryCode GetCountry() const;

    private:
      friend struct Beam::Serialization::Shuttle<Security>;
      friend std::size_t hash_value(const Security& security);
      std::string m_symbol;
      MarketCode m_market;
      CountryCode m_country;
  };

  //! Parses a Security.
  /*!
    \param source The string to parse.
    \param marketDatabase The database containing all MarketCodes.
    \return The Security represented by the <i>source</i>.
  */
  inline Security ParseSecurity(const std::string& source,
      const MarketDatabase& marketDatabase) {
    std::string::size_type seperator = source.find_last_of('.');
    if(seperator == std::string::npos) {
      return Security();
    }
    std::string symbol = source.substr(0, seperator);
    std::string marketSource = source.substr(seperator + 1);
    const MarketDatabase::Entry* market = &marketDatabase.FromDisplayName(
      marketSource);
    if(market->m_code == MarketCode()) {
      market = &marketDatabase.FromCode(marketSource);
      if(market->m_code == MarketCode()) {
        return Security();
      }
    }
    return Security(std::move(symbol), market->m_code, market->m_countryCode);
  }

  //! Parses a Security using the default MarketDatabase.
  /*!
    \param source The string to parse.
    \return The Security represented by the <i>source</i>.
  */
  inline Security ParseSecurity(const std::string& source) {
    return ParseSecurity(source, GetDefaultMarketDatabase());
  }

  inline std::string ToString(const Security& value,
      const MarketDatabase& marketDatabase) {
    if(value.GetMarket().IsEmpty() || value.GetSymbol().empty()) {
      return value.GetSymbol();
    }
    auto& market = marketDatabase.FromCode(value.GetMarket());
    if(market.m_code.IsEmpty()) {
      return (value.GetSymbol() + ".") + value.GetMarket().GetData();
    } else {
      return (value.GetSymbol() + ".") + market.m_displayName;
    }
  }

  inline std::string ToString(const Security& value) {
    return ToString(value, GetDefaultMarketDatabase());
  }

  inline std::ostream& operator <<(std::ostream& out, const Security& value) {
    return out << ToString(value);
  }

  inline std::istream& operator >>(std::istream& in, Security& value) {
    std::string s;
    in >> s;
    value = ParseSecurity(s);
    return in;
  }

  inline std::size_t hash_value(const Security& security) {
    std::size_t seed = 0;
    boost::hash_combine(seed, security.GetSymbol());
    boost::hash_combine(seed, security.GetCountry());
    return seed;
  }

  inline Security::Security()
      : m_country(CountryCode::NONE) {}

  inline Security::Security(std::string symbol, MarketCode market,
      CountryCode country)
      : m_symbol(std::move(symbol)),
        m_market(market),
        m_country(country) {}

  inline Security::Security(std::string symbol, CountryCode country)
      : m_symbol(std::move(symbol)),
        m_country(country) {}

  inline bool Security::operator <(const Security& rhs) const {
    return std::tie(m_symbol, m_country) <
      std::tie(rhs.m_symbol, rhs.m_country);
  }

  inline bool Security::operator ==(const Security& rhs) const {
    return m_symbol == rhs.m_symbol && m_country == rhs.m_country;
  }

  inline bool Security::operator !=(const Security& rhs) const {
    return !(*this == rhs);
  }

  inline const std::string& Security::GetSymbol() const {
    return m_symbol;
  }

  inline MarketCode Security::GetMarket() const {
    return m_market;
  }

  inline CountryCode Security::GetCountry() const {
    return m_country;
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::Security> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Security& value,
        unsigned int version) {
      shuttle.Shuttle("symbol", value.m_symbol);
      shuttle.Shuttle("market", value.m_market);
      shuttle.Shuttle("country", value.m_country);
    }
  };
}
}

namespace std {
  template <>
  struct hash<Nexus::Security> {
    size_t operator()(const Nexus::Security& value) const {
      return Nexus::hash_value(value);
    }
  };
};

#endif
