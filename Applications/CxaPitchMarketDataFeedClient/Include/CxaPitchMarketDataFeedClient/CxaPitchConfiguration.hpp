#ifndef CXA_PITCH_CONFIGURATION_HPP
#define CXA_PITCH_CONFIGURATION_HPP
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the addresses that a CXA PITCH feed is received on. */
  struct CxaPitchFeed {

    /** The name of the feed. */
    std::string m_name;

    /** The feed's real-time multicast group. */
    Beam::IpAddress m_address;

    /** The feed's gap response multicast group. */
    Beam::IpAddress m_gap_address;

    /** The interface to receive the feed on. */
    Beam::IpAddress m_interface;

    /**
     * Parses a CxaPitchFeed.
     * @param config The configuration to parse.
     * @param interface The interface to receive the feed on by default.
     * @return The CxaPitchFeed represented by the <i>config</i>.
     */
    static CxaPitchFeed parse(
      const YAML::Node& config, const Beam::IpAddress& interface);
  };

  /** Stores the credentials used to connect to a CXA PITCH server. */
  struct CxaPitchSession {

    /** The address of the server. */
    Beam::IpAddress m_address;

    /** The session sub id supplied by CXA. */
    std::string m_session_sub_id;

    /** The username supplied by CXA. */
    std::string m_username;

    /** The password supplied by CXA. */
    std::string m_password;

    /**
     * Parses a CxaPitchSession.
     * @param config The configuration to parse.
     * @return The CxaPitchSession represented by the <i>config</i>.
     */
    static CxaPitchSession parse(const YAML::Node& config);
  };

  /** Stores the configuration of a CXA PITCH market data parser. */
  struct CxaPitchConfiguration {

    /** Whether to log messages. */
    bool m_is_logging_messages;

    /** The unit that the feeds carry. */
    std::uint8_t m_unit;

    /** The venue's CountryCode. */
    CountryCode m_country;

    /** The venue that the Ticker is listed on. */
    Venue m_primary_venue;

    /** The venue disseminating the data. */
    Venue m_disseminating_venue;

    /** The MPID to display. */
    std::string m_mpid;

    /** The feeds to receive. */
    std::vector<CxaPitchFeed> m_feeds;

    /** How long a feed may be silent before it is excluded. */
    boost::posix_time::time_duration m_liveness;

    /** The gap request proxy to request missing messages from. */
    boost::optional<CxaPitchSession> m_retransmission;

    /** The spin server to request the state of the book from. */
    boost::optional<CxaPitchSession> m_spin;

    /**
     * Parses a CxaPitchConfiguration.
     * @param config The configuration to parse.
     * @return The CxaPitchConfiguration represented by the <i>config</i>.
     */
    static CxaPitchConfiguration parse(const YAML::Node& config);
  };

  inline CxaPitchFeed CxaPitchFeed::parse(
      const YAML::Node& config, const Beam::IpAddress& interface) {
    auto feed = CxaPitchFeed();
    feed.m_name = Beam::extract<std::string>(config, "name");
    feed.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    feed.m_gap_address = Beam::extract<Beam::IpAddress>(
      config, "gap_address", Beam::IpAddress());
    feed.m_interface =
      Beam::extract<Beam::IpAddress>(config, "interface", interface);
    return feed;
  }

  inline CxaPitchSession CxaPitchSession::parse(const YAML::Node& config) {
    auto session = CxaPitchSession();
    session.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    session.m_session_sub_id =
      Beam::extract<std::string>(config, "session_sub_id");
    session.m_username = Beam::extract<std::string>(config, "username");
    session.m_password = Beam::extract<std::string>(config, "password");
    return session;
  }

  inline CxaPitchConfiguration CxaPitchConfiguration::parse(
      const YAML::Node& config) {
    static const auto DEFAULT_LIVENESS = boost::posix_time::seconds(3);
    return Beam::try_or_nest([&] {
      auto configuration = CxaPitchConfiguration();
      configuration.m_is_logging_messages =
        Beam::extract<bool>(config, "enable_logging", false);
      configuration.m_unit =
        static_cast<std::uint8_t>(Beam::extract<int>(config, "unit"));
      auto primary_venue = VENUES.from_display_name(
        Beam::extract<std::string>(config, "venue"));
      if(!primary_venue.m_venue) {
        throw std::runtime_error("Unknown venue specified.");
      }
      configuration.m_country = primary_venue.m_country_code;
      configuration.m_primary_venue = primary_venue.m_venue;
      auto disseminating_venue = VENUES.from_display_name(
        Beam::extract<std::string>(config, "disseminating_venue"));
      if(!disseminating_venue.m_venue) {
        throw std::runtime_error("Unknown disseminating venue specified.");
      }
      configuration.m_disseminating_venue = disseminating_venue.m_venue;
      configuration.m_mpid = Beam::extract<std::string>(
        config, "mpid", disseminating_venue.m_display_name);
      auto interface = Beam::extract<Beam::IpAddress>(config, "interface");
      for(auto feed : Beam::get_node(config, "feeds")) {
        configuration.m_feeds.push_back(CxaPitchFeed::parse(feed, interface));
      }
      if(configuration.m_feeds.empty()) {
        throw std::runtime_error("No feeds specified.");
      }
      configuration.m_liveness =
        Beam::extract<boost::posix_time::time_duration>(
          config, "liveness", DEFAULT_LIVENESS);
      if(config["retransmission"]) {
        configuration.m_retransmission =
          CxaPitchSession::parse(Beam::get_node(config, "retransmission"));
      }
      if(config["spin"]) {
        configuration.m_spin =
          CxaPitchSession::parse(Beam::get_node(config, "spin"));
      }
      return configuration;
    }, std::runtime_error("Unable to parse the CXA PITCH configuration."));
  }
}

#endif
