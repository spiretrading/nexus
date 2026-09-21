#ifndef TMX_IP_CONFIGURATION_HPP
#define TMX_IP_CONFIGURATION_HPP
#include <limits>
#include <vector>
#include <Beam/Network/MulticastSocketOptions.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/StandardTimeZones.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"

namespace Nexus {

  /** The multicast endpoint for one TMX IP stream. */
  struct TmxIpFeed {

    /** The multicast group and destination port. */
    Beam::IpAddress m_address;

    /** The local interface receiving multicast data. */
    Beam::IpAddress m_interface;

    /** Parses a live feed's configuration. */
    static TmxIpFeed parse(const YAML::Node& config);
  };

  /** The recovery endpoints for the selected stream and site. */
  struct TmxIpRecoveryConfiguration {

    /** The recovery request server's address and port. */
    Beam::IpAddress m_address;

    /** The registered local interface used to send recovery requests. */
    Beam::IpAddress m_interface;

    /** The registered local address and port receiving recovered packets. */
    Beam::IpAddress m_delivery_address;

    /** The deadline for a recovery request and its packet delivery. */
    boost::posix_time::time_duration m_timeout;

    /** Parses the recovery configuration. */
    static TmxIpRecoveryConfiguration parse(const YAML::Node& config);
  };

  /** The configuration for receiving one TMX IP stream. */
  struct TmxIpConfiguration {

    /** Whether to log complete STAMP messages. */
    bool m_is_logging_messages;

    /** The country whose market data registry receives published updates. */
    CountryCode m_country;

    /** The CDF venue; unspecified for consolidated services. */
    Venue m_venue;

    /** The configured time zone, otherwise the venue's zone or Toronto. */
    boost::local_time::time_zone_ptr m_time_zone;

    /** The daily session rollover time in the transport's local time zone. */
    boost::posix_time::time_duration m_rollover_time;

    /** Identically sequenced copies of the live stream. */
    std::vector<TmxIpFeed> m_feeds;

    /** The socket options for live and recovered packets. */
    Beam::MulticastSocketOptions m_socket_options;

    /** The interval between publishing buffered market data updates. */
    boost::posix_time::time_duration m_sampling;

    /** How long a stalled feed participates in gap confirmation. */
    boost::posix_time::time_duration m_feed_timeout;

    /** How long to wait before logging and skipping an unrecovered gap. */
    boost::posix_time::time_duration m_gap_timeout;

    /** The interval between opportunities to retry or continue recovery. */
    boost::posix_time::time_duration m_retry_interval;

    /** The recovery endpoints for the live feed. */
    boost::optional<TmxIpRecoveryConfiguration> m_recovery;

    /** Parses the application's configuration. */
    static TmxIpConfiguration parse(const YAML::Node& config);
  };

namespace Details {
  inline Beam::IpAddress extract_tmx_ip_address(
      const YAML::Node& config, const char* name) {
    auto source = Beam::extract<std::string>(config, name);
    boost::trim(source);
    auto separator = source.find(':');
    if(separator != std::string::npos &&
        boost::lexical_cast<int>(source.substr(separator + 1)) < 0) {
      boost::throw_with_location(
        std::runtime_error("A port cannot be negative."));
    }
    return Beam::extract<Beam::IpAddress>(config, name);
  }
}

  inline TmxIpFeed TmxIpFeed::parse(const YAML::Node& config) {
    auto feed = TmxIpFeed();
    feed.m_address = Details::extract_tmx_ip_address(config, "address");
    if(feed.m_address.get_host().empty() || feed.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The feed address must specify a host and nonzero port."));
    }
    feed.m_interface = Details::extract_tmx_ip_address(config, "interface");
    if(feed.m_interface.get_host().empty()) {
      boost::throw_with_location(
        std::runtime_error("The feed interface must specify a host."));
    }
    return feed;
  }

  inline TmxIpRecoveryConfiguration TmxIpRecoveryConfiguration::parse(
      const YAML::Node& config) {
    auto recovery = TmxIpRecoveryConfiguration();
    recovery.m_address = Details::extract_tmx_ip_address(config, "address");
    if(recovery.m_address.get_host().empty() ||
        recovery.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The recovery server must specify a host and nonzero port."));
    }
    recovery.m_interface = Details::extract_tmx_ip_address(config, "interface");
    if(recovery.m_interface.get_host().empty()) {
      boost::throw_with_location(
        std::runtime_error("The recovery interface must specify a host."));
    }
    recovery.m_delivery_address =
      Details::extract_tmx_ip_address(config, "delivery_address");
    if(recovery.m_delivery_address.get_host().empty() ||
        recovery.m_delivery_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The recovery delivery address must specify a host and nonzero port."));
    }
    static const auto MAXIMUM_DURATION =
      boost::posix_time::time_duration(boost::date_time::max_date_time);
    recovery.m_timeout = Beam::extract<boost::posix_time::time_duration>(
      config, "timeout", boost::posix_time::seconds(30),
      boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
    return recovery;
  }

  inline TmxIpConfiguration TmxIpConfiguration::parse(
      const YAML::Node& config) {
    return Beam::try_or_nest([&] {
      auto configuration = TmxIpConfiguration();
      configuration.m_is_logging_messages =
        Beam::extract<bool>(config, "enable_logging", false);
      configuration.m_country =
        parse_country_code(Beam::extract<std::string>(config, "country", "CA"));
      if(!configuration.m_country) {
        boost::throw_with_location(
          std::runtime_error("Invalid market data country."));
      }
      if(config["venue"]) {
        configuration.m_venue =
          parse_venue(Beam::extract<std::string>(config, "venue"));
        if(!configuration.m_venue) {
          boost::throw_with_location(
            std::runtime_error("Invalid market data venue."));
        }
      }
      auto time_zone = [&] {
        if(config["time_zone"]) {
          return Beam::extract<std::string>(config, "time_zone");
        }
        if(configuration.m_venue) {
          return VENUES.from(configuration.m_venue).m_time_zone;
        }
        return std::string("America/Toronto");
      }();
      configuration.m_time_zone = TIME_ZONES.time_zone_from_region(time_zone);
      if(!configuration.m_time_zone) {
        boost::throw_with_location(
          std::runtime_error("Invalid TMX IP time zone: " + time_zone));
      }
      auto rollover =
        Beam::extract<std::string>(config, "rollover_time", "00:30:00");
      auto is_valid_rollover = [&] {
        if(rollover.size() < 8 || rollover[2] != ':' || rollover[5] != ':') {
          return false;
        }
        if(rollover.size() != 8 && (rollover.size() < 10 ||
            rollover[8] != '.' || rollover.size() > 9 +
              boost::posix_time::time_duration::num_fractional_digits())) {
          return false;
        }
        for(auto i = std::size_t(0); i != rollover.size(); ++i) {
          if(i != 2 && i != 5 && i != 8 &&
              (rollover[i] < '0' || rollover[i] > '9')) {
            return false;
          }
        }
        return rollover[3] < '6' && rollover[6] < '6';
      };
      if(!is_valid_rollover()) {
        boost::throw_with_location(
          std::runtime_error("Invalid TMX IP rollover time."));
      }
      configuration.m_rollover_time =
        boost::posix_time::duration_from_string(rollover);
      if(configuration.m_rollover_time.is_special() ||
          configuration.m_rollover_time < boost::posix_time::seconds(0) ||
          configuration.m_rollover_time >= boost::posix_time::hours(24)) {
        boost::throw_with_location(
          std::runtime_error("Invalid TMX IP rollover time."));
      }
      auto feeds = Beam::get_node(config, "feeds");
      if(!feeds.IsSequence() || feeds.size() == 0) {
        boost::throw_with_location(
          std::runtime_error("At least one live feed must be specified."));
      }
      for(auto feed : feeds) {
        configuration.m_feeds.push_back(TmxIpFeed::parse(feed));
      }
      static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE =
        std::size_t(128 * 1024 * 1024);
      configuration.m_socket_options.m_receive_buffer_size =
        Beam::extract<std::size_t>(
          config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE, std::size_t(1),
          std::size_t(std::numeric_limits<int>::max()));
      configuration.m_socket_options.m_enable_loopback = false;
      static constexpr auto MAXIMUM_DATAGRAM_SIZE =
        std::size_t(std::numeric_limits<std::uint16_t>::max());
      configuration.m_socket_options.m_max_datagram_size =
        MAXIMUM_DATAGRAM_SIZE;
      static const auto MAXIMUM_DURATION =
        boost::posix_time::time_duration(boost::date_time::max_date_time);
      configuration.m_sampling =
        Beam::extract<boost::posix_time::time_duration>(
          config, "sampling", boost::posix_time::milliseconds(100),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_feed_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "feed_timeout", boost::posix_time::seconds(1),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_gap_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "gap_timeout", boost::posix_time::seconds(1),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_retry_interval =
        Beam::extract<boost::posix_time::time_duration>(
          config, "retry_interval", boost::posix_time::seconds(1),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      if(auto recovery = config["recovery"]) {
        configuration.m_recovery = TmxIpRecoveryConfiguration::parse(recovery);
      }
      return configuration;
    }, std::runtime_error("Unable to parse the TMX IP configuration."));
  }
}

#endif
