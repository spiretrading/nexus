#ifndef NEXUS_CTACONFIGURATION_HPP
#define NEXUS_CTACONFIGURATION_HPP
#include <string>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct CtaConfiguration
      \brief Stores the configuration of a CTA parser.
   */
  struct CtaConfiguration {

    //! Whether to log messages.
    bool m_isLoggingMessages;

    //! The Country of origin.
    Nexus::CountryCode m_country;

    //! Maps single character codes to MarketCodes.
    std::array<MarketCode, 128> m_marketCodes;

    //! Parses a CtaConfiguration from a YAML Node.
    /*!
      \param config The YAML Node to parse.
      \param countryDatabase The set of valid countries.
      \param marketDatabase The set of valid markets.
      \return The CtaConfiguration represented by the <i>config</i>.
    */
    static CtaConfiguration Parse(const YAML::Node& config,
      const CountryDatabase& countryDatabase,
      const MarketDatabase& marketDatabase);
  };

  inline CtaConfiguration CtaConfiguration::Parse(const YAML::Node& config,
      const CountryDatabase& countryDatabase,
      const MarketDatabase& marketDatabase) {
    CtaConfiguration ctaConfig;
    ctaConfig.m_isLoggingMessages = Beam::Extract<bool>(config,
      "enable_logging", false);
    auto country = Beam::Extract<std::string>(config, "country", "US");
    auto countryCode = ParseCountryCode(country, countryDatabase);
    if(countryCode == CountryDatabase::NONE) {
      BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
        "Country not found.", config["country"].Mark()));
    }
    ctaConfig.m_country = countryCode;
    auto& marketCodes = Beam::GetNode(config, "market_codes");
    for(auto& marketCode : marketCodes) {
      auto code = Beam::Extract<std::string>(marketCode, "code");
      if(code.size() != 1) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
          "Invalid market code.", marketCode["code"].Mark()));
      }
      auto marketIdentifier = Beam::Extract<std::string>(marketCode, "market");
      auto entry = ParseMarketCode(marketIdentifier, marketDatabase);
      ctaConfig.m_marketCodes[code.front()] = entry;
    }
    return ctaConfig;
  }
}
}

#endif
