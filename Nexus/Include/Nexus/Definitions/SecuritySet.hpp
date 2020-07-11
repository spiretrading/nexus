#ifndef NEXUS_SECURITYSET_HPP
#define NEXUS_SECURITYSET_HPP
#include <unordered_set>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"

namespace Nexus {

  /*! \class SecuritySet
      \brief Represents a set of Securities, including wild-cards, that can be
             tested against.
   */
  class SecuritySet {
    public:

      //! A symbol that represents a wild-card.
      static std::string GetSymbolWildCard();

      //! A MarketCode that represents a wild-card.
      static MarketCode GetMarketCodeWildCard();

      //! A CountryCode that represents a wild-card.
      static CountryCode GetCountryCodeWildCard();

      //! A Security that represents a wild-card.
      static Security GetSecurityWildCard();

      //! A SecuritySet containing all Securities.
      static const SecuritySet& AllSecurities();

      //! Constructs an empty SecuritySet.
      SecuritySet() = default;

      //! Tests if this set is empty.
      bool IsEmpty() const;

      //! Tests if this set contains a <i>security</i>.
      /*!
        \param security The Security to test, it can not contains any
               wild-cards.
        \return <code>true</code> iff this set contains the <i>security</i>.
      */
      bool Contains(const Security& security) const;

      //! Adds a Security to this set.
      /*!
        \param security The Security to add.
      */
      void Add(Security security);

    private:
      std::unordered_set<Security> m_concreteSecurities;
      std::vector<Security> m_wildCards;
  };

  //! Parses a Security containing wild cards.
  /*!
    \param source The string to parse.
    \param marketDatabase The database containing all MarketCodes.
    \return The Security represented by the <i>source</i>.
  */
  inline boost::optional<Security> ParseWildCardSecurity(
      const std::string& source, const MarketDatabase& marketDatabase,
      const CountryDatabase& countryDatabase) {
    if(source == "*" || source == "*.*" || source == "*.*.*") {
      return Security{SecuritySet::GetSymbolWildCard(),
        SecuritySet::GetMarketCodeWildCard(),
        SecuritySet::GetCountryCodeWildCard()};
    }
    auto seperator = source.find_last_of('.');
    if(seperator == std::string::npos) {
      return boost::none;
    }
    auto header = source.substr(0, seperator);
    auto trailer = source.substr(seperator + 1);
    if(header == SecuritySet::GetSymbolWildCard()) {
      auto& market = ParseMarketEntry(trailer, marketDatabase);
      if(market.m_code != MarketCode{}) {
        return Security{header, market.m_code, market.m_countryCode};
      }
    }
    auto prefixSecurity = ParseWildCardSecurity(header, marketDatabase,
      countryDatabase);
    if(prefixSecurity.is_initialized()) {
      if(trailer.size() == 2) {
        auto code = countryDatabase.FromTwoLetterCode(trailer);
        if(code.m_code != CountryCode::NONE) {
          return Security{prefixSecurity->GetSymbol(),
            prefixSecurity->GetMarket(), code.m_code};
        } else {
          return boost::none;
        }
      } else if(trailer == "*") {
        return Security{prefixSecurity->GetSymbol(),
          prefixSecurity->GetMarket(), SecuritySet::GetCountryCodeWildCard()};
      } else {
        return boost::none;
      }
    }
    MarketCode market;
    CountryCode country;
    if(trailer == "*") {
      market = SecuritySet::GetMarketCodeWildCard();
      country = SecuritySet::GetCountryCodeWildCard();
    } else {
      auto& marketEntry = ParseMarketEntry(trailer, marketDatabase);
      if(marketEntry.m_code == MarketCode{}) {
        return boost::none;
      }
      market = marketEntry.m_code;
      country = marketEntry.m_countryCode;
    }
    return Security{std::move(header), market, country};
  }

  //! Returns the string representation of a Security, including wild-cards.
  /*!
    \param security The Security to represent.
    \param marketDatabase The MarketDatabase used to represent the MarketCode.
    \param countryDatabase The CountryDatabase used to represent the
           CountryCode.
    \return The string representation of the <i>security</i>.
  */
  inline std::string ToWildCardString(const Security& security,
      const MarketDatabase& marketDatabase,
      const CountryDatabase& countryDatabase) {
    if(security.GetSymbol() == SecuritySet::GetSymbolWildCard() &&
        security.GetMarket() == SecuritySet::GetMarketCodeWildCard() &&
        security.GetCountry() == SecuritySet::GetCountryCodeWildCard()) {
      return "*";
    } else if(security == Security{}) {
      return "";
    }
    auto symbol = security.GetSymbol() + ".";
    if(security.GetMarket() == SecuritySet::GetMarketCodeWildCard()) {
      if(security.GetCountry() != SecuritySet::GetCountryCodeWildCard()) {
        auto& countryEntry = countryDatabase.FromCode(security.GetCountry());
        symbol += countryEntry.m_twoLetterCode.GetData();
      } else {
        symbol += "*";
      }
      return symbol;
    } else {
      auto& market = marketDatabase.FromCode(security.GetMarket());
      symbol += market.m_displayName;
    }
    return symbol;
  }

  inline std::string SecuritySet::GetSymbolWildCard() {
    return {"*"};
  }

  inline MarketCode SecuritySet::GetMarketCodeWildCard() {
    return {"*"};
  }

  inline CountryCode SecuritySet::GetCountryCodeWildCard() {
    return CountryCode::NONE;
  }

  inline Security SecuritySet::GetSecurityWildCard() {
    return {GetSymbolWildCard(), GetMarketCodeWildCard(),
      GetCountryCodeWildCard()};
  }

  inline const SecuritySet& SecuritySet::AllSecurities() {
    static auto value =
      [] {
        SecuritySet value;
        value.Add(SecuritySet::GetSecurityWildCard());
        return value;
      }();
    return value;
  }

  inline bool SecuritySet::IsEmpty() const {
    return m_wildCards.empty() && m_concreteSecurities.empty();
  }

  inline bool SecuritySet::Contains(const Security& security) const {
    for(auto& wildCard : m_wildCards) {
      if((wildCard.GetSymbol() == GetSymbolWildCard() ||
          wildCard.GetSymbol() == security.GetSymbol()) &&
          (wildCard.GetMarket() == GetMarketCodeWildCard() ||
          wildCard.GetMarket() == security.GetMarket()) &&
          (wildCard.GetCountry() == GetCountryCodeWildCard() ||
          wildCard.GetCountry() == security.GetCountry())) {
        return true;
      }
    }
    return m_concreteSecurities.find(security) != m_concreteSecurities.end();
  }

  inline void SecuritySet::Add(Security security) {
    if(security.GetSymbol() == GetSymbolWildCard() ||
        security.GetMarket() == GetMarketCodeWildCard() ||
        security.GetCountry() == GetCountryCodeWildCard()) {
      m_wildCards.push_back(std::move(security));
    } else {
      m_concreteSecurities.insert(std::move(security));
    }
  }
}

#endif
