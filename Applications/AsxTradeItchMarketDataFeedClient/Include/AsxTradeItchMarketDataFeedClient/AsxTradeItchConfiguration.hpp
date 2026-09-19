#ifndef ASX_TRADE_ITCH_CONFIGURATION_HPP
#define ASX_TRADE_ITCH_CONFIGURATION_HPP
#include <algorithm>
#include <limits>
#include <Beam/Network/MulticastSocketOptions.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores the addresses used to receive one multicast feed. */
  struct AsxTradeItchFeed {

    /** The name of the feed. */
    std::string m_name;

    /** The multicast group and destination port. */
    Beam::IpAddress m_address;

    /** The local interface to receive the feed on. */
    Beam::IpAddress m_interface;

    /** Parses a multicast feed's configuration. */
    static AsxTradeItchFeed parse(const YAML::Node& config);
  };

  /** Stores the connection settings for a partition's rewind server. */
  struct AsxTradeItchRewindServer {

    /** The server's UDP address. */
    Beam::IpAddress m_address;

    /** The local address used to send requests and receive replies. */
    Beam::IpAddress m_interface;

    /** Parses a rewind server's configuration. */
    static AsxTradeItchRewindServer parse(const YAML::Node& config);
  };

  /** Stores the connection settings for a partition's Glimpse server. */
  struct AsxTradeItchGlimpseServer {

    /** The server's TCP address. */
    Beam::IpAddress m_address;

    /** The local interface to connect from. */
    Beam::IpAddress m_interface;

    /** The username supplied by ASX. */
    std::string m_username;

    /** The password supplied by ASX. */
    std::string m_password;

    /** Parses a Glimpse server's configuration. */
    static AsxTradeItchGlimpseServer parse(const YAML::Node& config);
  };

  /** Stores the configuration of one ASX Trade ITCH partition. */
  struct AsxTradeItchConfiguration {

    /** Whether to log received messages. */
    bool m_is_logging_messages;

    /** The partition carried by the configured endpoints. */
    int m_partition;

    /** The venue's country. */
    CountryCode m_country;

    /** The venue that securities are listed on. */
    Venue m_primary_venue;

    /** The venue disseminating the data. */
    Venue m_disseminating_venue;

    /** The MPID to display. */
    std::string m_mpid;

    /** The redundant multicast feeds to receive. */
    std::vector<AsxTradeItchFeed> m_feeds;

    /** The socket options for multicast and rewind datagrams. */
    Beam::MulticastSocketOptions m_socket_options;

    /** The interval between publishing sampled market data. */
    boost::posix_time::time_duration m_sampling;

    /** How long a feed may remain silent or stalled. */
    boost::posix_time::time_duration m_feed_timeout;

    /** How long to wait before retrying a rewind request. */
    boost::posix_time::time_duration m_request_timeout;

    /** The server to request missing messages from. */
    AsxTradeItchRewindServer m_rewind;

    /** The optional server to load the startup snapshot from. */
    boost::optional<AsxTradeItchGlimpseServer> m_glimpse;

    /** Parses the partition's configuration. */
    static AsxTradeItchConfiguration parse(const YAML::Node& config);

    /** Returns the polling interval for feed expiry and recovery retries. */
    boost::posix_time::time_duration get_timer_interval() const;
  };

  inline AsxTradeItchFeed AsxTradeItchFeed::parse(const YAML::Node& config) {
    auto feed = AsxTradeItchFeed();
    feed.m_name = Beam::extract<std::string>(config, "name");
    feed.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    if(feed.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The feed address must specify a nonzero port."));
    }
    feed.m_interface = Beam::extract<Beam::IpAddress>(config, "interface");
    return feed;
  }

  inline AsxTradeItchRewindServer AsxTradeItchRewindServer::parse(
      const YAML::Node& config) {
    auto server = AsxTradeItchRewindServer();
    server.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    if(server.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The rewind address must specify a nonzero port."));
    }
    server.m_interface = Beam::extract<Beam::IpAddress>(config, "interface");
    return server;
  }

  inline AsxTradeItchGlimpseServer AsxTradeItchGlimpseServer::parse(
      const YAML::Node& config) {
    auto server = AsxTradeItchGlimpseServer();
    server.m_address = Beam::extract<Beam::IpAddress>(config, "address");
    if(server.m_address.get_port() == 0) {
      boost::throw_with_location(std::runtime_error(
        "The Glimpse address must specify a nonzero port."));
    }
    server.m_interface = Beam::extract<Beam::IpAddress>(config, "interface");
    server.m_username = Beam::extract<std::string>(config, "username");
    server.m_password = Beam::extract<std::string>(config, "password");
    static constexpr auto USERNAME_LENGTH = std::size_t(6);
    static constexpr auto PASSWORD_LENGTH = std::size_t(10);
    if(server.m_username.empty() ||
        server.m_username.size() > USERNAME_LENGTH ||
        server.m_password.empty() ||
        server.m_password.size() > PASSWORD_LENGTH) {
      boost::throw_with_location(
        std::runtime_error("Invalid Glimpse credential lengths."));
    }
    return server;
  }

  inline AsxTradeItchConfiguration AsxTradeItchConfiguration::parse(
      const YAML::Node& config) {
    return Beam::try_or_nest([&] {
      auto configuration = AsxTradeItchConfiguration();
      configuration.m_is_logging_messages =
        Beam::extract<bool>(config, "enable_logging", false);
      configuration.m_partition = Beam::extract<int>(config, "partition", 1, 4);
      auto& primary_venue =
        VENUES.from_display_name(Beam::extract<std::string>(config, "venue"));
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
      auto feeds = Beam::get_node(config, "feeds");
      if(!feeds.IsSequence() || feeds.size() == 0) {
        boost::throw_with_location(
          std::runtime_error("A nonempty list of feeds is required."));
      }
      for(auto feed : feeds) {
        configuration.m_feeds.push_back(AsxTradeItchFeed::parse(feed));
      }
      static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE =
        std::size_t(128 * 1024 * 1024);
      static constexpr auto MAXIMUM_DATAGRAM_SIZE =
        std::size_t(std::numeric_limits<std::uint16_t>::max());
      configuration.m_socket_options.m_receive_buffer_size =
        Beam::extract<std::size_t>(config, "receive_buffer",
          DEFAULT_RECEIVE_BUFFER_SIZE, std::size_t(1),
          std::size_t(std::numeric_limits<int>::max()));
      configuration.m_socket_options.m_max_datagram_size =
        Beam::extract<std::size_t>(config, "mtu", MAXIMUM_DATAGRAM_SIZE,
          std::size_t(1500), MAXIMUM_DATAGRAM_SIZE);
      static const auto MAXIMUM_DURATION =
        boost::posix_time::time_duration(boost::date_time::max_date_time);
      configuration.m_sampling =
        Beam::extract<boost::posix_time::time_duration>(config, "sampling",
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_feed_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "feed_timeout", boost::posix_time::seconds(3),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_request_timeout =
        Beam::extract<boost::posix_time::time_duration>(
          config, "request_timeout", boost::posix_time::seconds(1),
          boost::posix_time::time_duration::unit(), MAXIMUM_DURATION);
      configuration.m_rewind = AsxTradeItchRewindServer::parse(
        Beam::get_node(config, "rewind"));
      if(config["glimpse"]) {
        configuration.m_glimpse = AsxTradeItchGlimpseServer::parse(
          Beam::get_node(config, "glimpse"));
      }
      return configuration;
    }, std::runtime_error("Unable to parse the ASX Trade ITCH configuration."));
  }

  inline boost::posix_time::time_duration
      AsxTradeItchConfiguration::get_timer_interval() const {
    return std::min({boost::posix_time::time_duration(
      boost::posix_time::milliseconds(100)), m_feed_timeout,
      m_request_timeout});
  }
}

#endif
