#ifndef CXA_PITCH_CONFIGURATION_HPP
#define CXA_PITCH_CONFIGURATION_HPP
#include <algorithm>
#include <limits>
#include <Beam/Network/MulticastSocketOptions.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the addresses that a CXA PITCH feed is received on. */
  struct CxaPitchFeed {

    /** The feed's real-time multicast group. */
    Beam::IpAddress m_address;

    /** The feed's gap response multicast group. */
    boost::optional<Beam::IpAddress> m_gap_address;

    /** The interface to receive the feed on. */
    Beam::IpAddress m_interface;

    /**
     * Parses a CxaPitchFeed.
     * @param config The configuration to parse.
     * @return The CxaPitchFeed represented by the <i>config</i>.
     */
    static CxaPitchFeed parse(const YAML::Node& config);
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

    /** The socket options used to receive multicast data. */
    Beam::MulticastSocketOptions m_socket_options;

    /** The interval between publishing sampled market data. */
    boost::posix_time::time_duration m_sampling;

    /** How long a feed may be silent before it is excluded. */
    boost::posix_time::time_duration m_feed_timeout;

    /** How long to wait for a missing message before skipping over it. */
    boost::posix_time::time_duration m_gap_timeout;

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

    /** Returns the polling interval for timeouts and recovery retries. */
    boost::posix_time::time_duration get_timer_interval() const;
  };

  inline CxaPitchFeed CxaPitchFeed::parse(const YAML::Node& config) {
    auto feed = CxaPitchFeed();
    feed.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    if(feed.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The feed address must specify a nonzero port."));
    }
    if(config["gap_address"]) {
      feed.m_gap_address =
        Beam::extract<Beam::IpAddress>(config, "gap_address");
      if(feed.m_gap_address->get_port() == 0) {
        boost::throw_with_location(std::runtime_error(
          "The gap response address must specify a nonzero port."));
      }
    }
    feed.m_interface = Beam::extract<Beam::IpAddress>(config, "interface");
    return feed;
  }

  inline CxaPitchSession CxaPitchSession::parse(const YAML::Node& config) {
    auto session = CxaPitchSession();
    session.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    session.m_session_sub_id =
      Beam::extract<std::string>(config, "session_sub_id");
    session.m_username = Beam::extract<std::string>(config, "username");
    session.m_password = Beam::extract<std::string>(config, "password");
    constexpr auto SESSION_SUB_ID_LENGTH = 4;
    constexpr auto USERNAME_LENGTH = 4;
    constexpr auto PASSWORD_LENGTH = 10;
    if(session.m_session_sub_id.size() > SESSION_SUB_ID_LENGTH ||
        session.m_username.size() > USERNAME_LENGTH ||
        session.m_password.size() > PASSWORD_LENGTH) {
      boost::throw_with_location(std::runtime_error(
        "CXA PITCH session credentials exceed their field lengths."));
    }
    return session;
  }

  inline CxaPitchConfiguration CxaPitchConfiguration::parse(
      const YAML::Node& config) {
    return Beam::try_or_nest([&] {
      static const auto DEFAULT_FEED_TIMEOUT = boost::posix_time::seconds(3);
      static const auto DEFAULT_GAP_TIMEOUT = boost::posix_time::seconds(5);
      static const auto MAXIMUM_DURATION =
        boost::posix_time::time_duration(boost::date_time::max_date_time);
      auto configuration = CxaPitchConfiguration();
      configuration.m_is_logging_messages =
        Beam::extract<bool>(config, "enable_logging", false);
      configuration.m_unit = static_cast<std::uint8_t>(Beam::extract<int>(
        config, "unit", 1, std::numeric_limits<std::uint8_t>::max()));
      auto& primary_venue = VENUES.from_display_name(
        Beam::extract<std::string>(config, "venue"));
      if(!primary_venue.m_venue) {
        boost::throw_with_location(
          std::runtime_error("Unknown venue specified."));
      }
      configuration.m_country = primary_venue.m_country_code;
      configuration.m_primary_venue = primary_venue.m_venue;
      auto& disseminating_venue = VENUES.from_display_name(
        Beam::extract<std::string>(config, "disseminating_venue"));
      if(!disseminating_venue.m_venue) {
        boost::throw_with_location(
          std::runtime_error("Unknown disseminating venue specified."));
      }
      configuration.m_disseminating_venue = disseminating_venue.m_venue;
      configuration.m_mpid = Beam::extract<std::string>(
        config, "mpid", disseminating_venue.m_display_name);
      for(auto feed : Beam::get_node(config, "feeds")) {
        configuration.m_feeds.push_back(CxaPitchFeed::parse(feed));
      }
      if(configuration.m_feeds.empty()) {
        boost::throw_with_location(std::runtime_error("No feeds specified."));
      }
      static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE =
        std::size_t(128 * 1024 * 1024);
      static constexpr auto MINIMUM_DATAGRAM_SIZE = std::size_t(1500);
      configuration.m_socket_options.m_receive_buffer_size =
        Beam::extract<std::size_t>(config, "receive_buffer",
          DEFAULT_RECEIVE_BUFFER_SIZE, std::size_t(1),
          std::size_t(std::numeric_limits<int>::max()));
      configuration.m_socket_options.m_max_datagram_size =
        Beam::extract<std::size_t>(config, "mtu",
          configuration.m_socket_options.m_max_datagram_size,
          MINIMUM_DATAGRAM_SIZE,
          std::size_t(std::numeric_limits<std::uint16_t>::max()));
      configuration.m_sampling =
        Beam::extract<boost::posix_time::time_duration>(config, "sampling",
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_feed_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "feed_timeout", DEFAULT_FEED_TIMEOUT,
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_gap_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "gap_timeout", DEFAULT_GAP_TIMEOUT,
          boost::posix_time::time_duration(), MAXIMUM_DURATION);
      if(config["retransmission"]) {
        configuration.m_retransmission =
          CxaPitchSession::parse(Beam::get_node(config, "retransmission"));
      }
      if(configuration.m_retransmission &&
          std::ranges::none_of(configuration.m_feeds, [] (const auto& feed) {
            return feed.m_gap_address.has_value();
          })) {
        boost::throw_with_location(std::runtime_error(
          "No gap response address specified to receive retransmissions."));
      }
      if(config["spin"]) {
        configuration.m_spin =
          CxaPitchSession::parse(Beam::get_node(config, "spin"));
      }
      return configuration;
    }, std::runtime_error("Unable to parse the CXA PITCH configuration."));
  }

  inline boost::posix_time::time_duration
      CxaPitchConfiguration::get_timer_interval() const {
    auto interval = std::min<boost::posix_time::time_duration>(
      boost::posix_time::milliseconds(100), m_feed_timeout);
    if(m_gap_timeout > boost::posix_time::time_duration()) {
      interval = std::min(interval, m_gap_timeout);
    }
    return interval;
  }
}

#endif
