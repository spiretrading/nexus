#ifndef NEXUS_UTPCONFIGURATION_HPP
#define NEXUS_UTPCONFIGURATION_HPP
#include <string>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct UtpConfiguration
      \brief Stores the configuration of a UTP parser.
   */
  struct UtpConfiguration {

    //! Whether to log messages.
    bool m_isLoggingMessages;

    //! The Market disseminating the data.
    Nexus::MarketCode m_market;

    //! The Country of origin.
    Nexus::CountryCode m_country;

    //! Maps single character codes to MarketCodes.
    std::array<MarketCode, 128> m_marketCodes;

    //! Parses a UtpConfiguration from a YAML Node.
    /*!
      \param config The YAML Node to parse.
      \param marketDatabase The set of valid markets.
      \return The UtpConfiguration represented by the <i>config</i>.
    */
    static UtpConfiguration Parse(const YAML::Node& config,
      const MarketDatabase& marketDatabase);
  };

  inline UtpConfiguration UtpConfiguration::Parse(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    UtpConfiguration utpConfig;
    utpConfig.m_isLoggingMessages = Beam::Extract<bool>(config,
      "enable_logging", false);
    auto market = Beam::Extract<std::string>(config, "market");
    auto marketEntry = ParseMarketEntry(market, marketDatabase);
    utpConfig.m_country = marketEntry.m_countryCode;
    utpConfig.m_market = marketEntry.m_code;
    auto& marketCodes = Beam::GetNode(config, "market_codes");
    for(auto& marketCode : marketCodes) {
      auto code = Beam::Extract<std::string>(marketCode, "code");
      if(code.size() != 1) {
        BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
          "Invalid market code.", marketCode.FindValue("code")->GetMark()));
      }
      auto marketIdentifier = Beam::Extract<std::string>(marketCode, "market");
      auto entry = ParseMarketCode(marketIdentifier, marketDatabase);
      utpConfig.m_marketCodes[code.front()] = entry;
    }
    return utpConfig;
  }
}
}

#endif
