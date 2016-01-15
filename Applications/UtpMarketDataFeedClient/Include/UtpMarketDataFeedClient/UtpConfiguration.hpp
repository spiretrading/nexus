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

    //! The time used as the origin of market data messages disseminated by
    //! market data server.
    boost::posix_time::ptime m_timeOrigin;

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
      \param currentTime The current time used to establish the time origin.
      \param timeZones The list of time zones.
      \return The UtpConfiguration represented by the <i>config</i>.
    */
    static UtpConfiguration Parse(const YAML::Node& config,
      const MarketDatabase& marketDatabase,
      const boost::posix_time::ptime& currentTime,
      const boost::local_time::tz_database& timeZones);
  };

  inline UtpConfiguration UtpConfiguration::Parse(const YAML::Node& config,
      const MarketDatabase& marketDatabase,
      const boost::posix_time::ptime& currentTime,
      const boost::local_time::tz_database& timeZones) {
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
    auto configTimezone = Beam::Extract<std::string>(config, "time_zone",
      "Eastern_Time");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
        "Time zone not found.", config.FindValue("time_zone")->GetMark()));
    }
    boost::posix_time::ptime serverDate{
      Beam::TimeService::AdjustDateTime(currentTime, "UTC", configTimezone,
      timeZones).date(), boost::posix_time::seconds(0)};
    utpConfig.m_timeOrigin = Beam::TimeService::AdjustDateTime(serverDate,
      configTimezone, "UTC", timeZones);
    return utpConfig;
  }
}
}

#endif
