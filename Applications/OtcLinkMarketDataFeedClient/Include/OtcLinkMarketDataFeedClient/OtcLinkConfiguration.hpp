#ifndef OTC_LINK_CONFIGURATION_HPP
#define OTC_LINK_CONFIGURATION_HPP
#include <algorithm>
#include <charconv>
#include <limits>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketOptions.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/StandardCountries.hpp"

namespace Nexus {

  /** The multicast endpoint for one OTC Link feed. */
  struct OtcLinkFeed {

    /** The multicast group and destination port. */
    Beam::IpAddress m_address;

    /** The local interface receiving the feed. */
    Beam::IpAddress m_interface;

    /** Parses a multicast feed's configuration. */
    static OtcLinkFeed parse(const YAML::Node& config);
  };

  /** The server accepting gap and snapshot requests. */
  struct OtcLinkRecoveryConfiguration {

    /** The server's address and port. */
    Beam::IpAddress m_address;

    /** The local interface used to connect. */
    Beam::IpAddress m_interface;

    /** The subscriber's assigned SenderCompID. */
    std::string m_sender;

    /** The deadline for an individual request. */
    boost::posix_time::time_duration m_timeout;

    /** Parses a recovery server's configuration. */
    static OtcLinkRecoveryConfiguration parse(const YAML::Node& config);
  };

  /** The endpoints used to load an initial snapshot. */
  struct OtcLinkSnapshotConfiguration {

    /** The single multicast feed receiving the snapshot. */
    OtcLinkFeed m_feed;

    /** The server publishing the selected snapshot feed. */
    OtcLinkRecoveryConfiguration m_server;

    /** The deadline for acknowledgement and the complete snapshot. */
    boost::posix_time::time_duration m_timeout;

    /** Parses a snapshot's configuration. */
    static OtcLinkSnapshotConfiguration parse(const YAML::Node& config);
  };

  /** The configuration for an OTC Link Book or Inside service. */
  struct OtcLinkConfiguration {

    /** The country whose registry receives published market data. */
    CountryCode m_country;

    /** The real-time channel's RefApplID. */
    std::uint16_t m_channel;

    /** The redundant live feeds for the selected service. */
    std::vector<OtcLinkFeed> m_feeds;

    /** The socket options for live and snapshot datagrams. */
    Beam::MulticastSocketOptions m_socket_options;

    /** The interval between publishing sampled market data. */
    boost::posix_time::time_duration m_sampling;

    /** How long a silent or stalled feed delays gap confirmation. */
    boost::posix_time::time_duration m_feed_timeout;

    /** How long to wait for live messages or an outstanding recovery. */
    boost::posix_time::time_duration m_gap_timeout;

    /** The optional server used to recover missing live messages. */
    boost::optional<OtcLinkRecoveryConfiguration> m_recovery;

    /** The optional initial snapshot. */
    boost::optional<OtcLinkSnapshotConfiguration> m_snapshot;

    /** Parses the application's configuration. */
    static OtcLinkConfiguration parse(const YAML::Node& config);

    /** Returns the interval for feed expiry and gap deadlines. */
    boost::posix_time::time_duration get_timer_interval() const;
  };

namespace Details {
  inline Beam::IpAddress parse_otc_link_address(
      const YAML::Node& config, const char* key, bool is_port_required) {
    auto text = Beam::extract<std::string>(config, key);
    auto separator = text.find(':');
    auto host = text.substr(0, separator);
    auto port = 0U;
    auto is_valid = !host.empty() &&
      host.find_first_of(" \t\r\n") == std::string::npos;
    if(separator != std::string::npos) {
      auto end = text.data() + text.size();
      auto result =
        std::from_chars(text.data() + separator + 1, end, port);
      is_valid = is_valid && result.ec == std::errc() && result.ptr == end;
    }
    if(!is_valid || port > std::numeric_limits<std::uint16_t>::max() ||
        (is_port_required && port == 0)) {
      boost::throw_with_location(
        std::runtime_error(std::string("Invalid OTC Link ") + key + '.'));
    }
    return Beam::IpAddress(std::move(host), static_cast<std::uint16_t>(port));
  }

  inline boost::posix_time::time_duration parse_otc_link_duration(
      const YAML::Node& config, const char* key,
      boost::posix_time::time_duration fallback) {
    return Beam::extract<boost::posix_time::time_duration>(config, key,
      fallback, boost::posix_time::time_duration::unit(),
      boost::posix_time::time_duration(boost::date_time::max_date_time));
  }
}

  inline OtcLinkFeed OtcLinkFeed::parse(const YAML::Node& config) {
    return OtcLinkFeed(
      Details::parse_otc_link_address(config, "address", true),
      Details::parse_otc_link_address(config, "interface", false));
  }

  inline OtcLinkRecoveryConfiguration OtcLinkRecoveryConfiguration::parse(
      const YAML::Node& config) {
    auto server = OtcLinkRecoveryConfiguration();
    server.m_address =
      Details::parse_otc_link_address(config, "address", true);
    server.m_interface =
      Details::parse_otc_link_address(config, "interface", false);
    server.m_sender = Beam::extract<std::string>(config, "sender");
    if(server.m_sender.empty() ||
        server.m_sender.find('\x01') != std::string::npos) {
      boost::throw_with_location(std::runtime_error("Invalid SenderCompID."));
    }
    server.m_timeout = Details::parse_otc_link_duration(
      config, "timeout", boost::posix_time::seconds(1));
    return server;
  }

  inline OtcLinkSnapshotConfiguration OtcLinkSnapshotConfiguration::parse(
      const YAML::Node& config) {
    return OtcLinkSnapshotConfiguration(OtcLinkFeed::parse(config),
      OtcLinkRecoveryConfiguration::parse(Beam::get_node(config, "server")),
      Details::parse_otc_link_duration(
        config, "timeout", boost::posix_time::seconds(30)));
  }

  inline OtcLinkConfiguration OtcLinkConfiguration::parse(
      const YAML::Node& config) {
    auto configuration = OtcLinkConfiguration();
    configuration.m_country =
      parse_country_code(Beam::extract<std::string>(config, "country", "US"));
    if(!configuration.m_country) {
      boost::throw_with_location(std::runtime_error("Unknown country."));
    }
    auto service = Beam::extract<std::string>(config, "service");
    if(service == "book") {
      configuration.m_channel = 11;
    } else if(service == "inside") {
      configuration.m_channel = 14;
    } else {
      boost::throw_with_location(std::runtime_error("Unknown OTC service."));
    }
    auto feeds = Beam::get_node(config, "feeds");
    if(!feeds.IsSequence() || feeds.size() == 0) {
      boost::throw_with_location(
        std::runtime_error("A nonempty list of feeds is required."));
    }
    for(auto feed : feeds) {
      configuration.m_feeds.push_back(OtcLinkFeed::parse(feed));
    }
    static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE =
      std::size_t(128 * 1024 * 1024);
    configuration.m_socket_options.m_receive_buffer_size =
      Beam::extract<std::size_t>(config, "receive_buffer",
        DEFAULT_RECEIVE_BUFFER_SIZE, std::size_t(1),
        std::size_t(std::numeric_limits<int>::max()));
    configuration.m_socket_options.m_max_datagram_size =
      std::numeric_limits<std::uint16_t>::max();
    configuration.m_sampling = Details::parse_otc_link_duration(
      config, "sampling", boost::posix_time::milliseconds(100));
    configuration.m_feed_timeout = Details::parse_otc_link_duration(
      config, "feed_timeout", boost::posix_time::seconds(1));
    configuration.m_gap_timeout = Details::parse_otc_link_duration(
      config, "gap_timeout", boost::posix_time::seconds(1));
    if(auto recovery = config["recovery"]) {
      configuration.m_recovery = OtcLinkRecoveryConfiguration::parse(recovery);
    }
    if(auto snapshot = config["snapshot"]) {
      configuration.m_snapshot = OtcLinkSnapshotConfiguration::parse(snapshot);
    }
    return configuration;
  }

  inline boost::posix_time::time_duration
      OtcLinkConfiguration::get_timer_interval() const {
    return std::min({boost::posix_time::time_duration(
      boost::posix_time::milliseconds(100)), m_feed_timeout, m_gap_timeout});
  }
}

#endif
