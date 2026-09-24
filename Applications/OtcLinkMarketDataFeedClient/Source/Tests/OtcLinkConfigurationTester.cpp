#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkConfiguration.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  YAML::Node make_config() {
    return YAML::Load(R"(
service: book
feeds:
  - address: "224.0.23.210:21000"
    interface: "10.0.0.1:21000"
  - address: "224.0.23.227:21000"
    interface: "10.0.0.1:21000"
)");
  }
}

TEST_SUITE("OtcLinkConfiguration") {
  TEST_CASE("trade_channel") {
    auto source = make_config();
    REQUIRE_FALSE(OtcLinkConfiguration::parse(source).m_reference.has_value());
    source["reference"] = YAML::Load(R"(
address: "224.0.23.214:21005"
interface: "10.0.0.1:21005"
server:
  address: "192.26.98.86:21100"
  interface: "10.0.0.1:0"
  sender: subscriber
)");
    source["service"] = "trades";
    source["feeds"] = YAML::Load(R"(
- address: "224.0.23.218:21008"
  interface: "10.0.0.1:21008"
- address: "224.0.23.217:21008"
  interface: "10.0.0.1:21008"
)");
    auto config = OtcLinkConfiguration::parse(source);
    REQUIRE(config.m_channel == 1);
    REQUIRE(config.m_feeds.size() == 2);
    REQUIRE(config.m_feeds[0].m_address == IpAddress("224.0.23.218", 21008));
    REQUIRE(config.m_reference.has_value());
    REQUIRE(config.m_reference->m_feed.m_address ==
      IpAddress("224.0.23.214", 21005));
    REQUIRE(config.m_reference->m_server.m_sender == "subscriber");
    REQUIRE_FALSE(config.m_snapshot.has_value());
    SUBCASE("trade_snapshot") {
      source["snapshot"] = YAML::Load("{}");
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("reference_service") {
      for(auto service : {"book", "inside"}) {
        source["service"] = service;
        REQUIRE_THROWS_AS(
          OtcLinkConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("missing_reference") {
      source.remove("reference");
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("missing_reference_server") {
      source["reference"].remove("server");
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
  }

  TEST_CASE("defaults") {
    auto config = OtcLinkConfiguration::parse(make_config());
    REQUIRE(config.m_country == Countries::US);
    REQUIRE(config.m_channel == 11);
    REQUIRE(config.m_feeds.size() == 2);
    REQUIRE(config.m_feeds[0].m_address == IpAddress("224.0.23.210", 21000));
    REQUIRE(config.m_feeds[1].m_address == IpAddress("224.0.23.227", 21000));
    REQUIRE(config.m_feeds[0].m_interface == IpAddress("10.0.0.1", 21000));
    REQUIRE(config.m_sampling == milliseconds(100));
    REQUIRE(config.m_feed_timeout == seconds(1));
    REQUIRE(config.m_gap_timeout == seconds(1));
    REQUIRE(config.get_timer_interval() == milliseconds(100));
    REQUIRE(config.m_socket_options.m_receive_buffer_size ==
      128 * 1024 * 1024);
    REQUIRE(config.m_socket_options.m_max_datagram_size == 65535);
    REQUIRE_FALSE(config.m_recovery.has_value());
    REQUIRE_FALSE(config.m_snapshot.has_value());
  }

  TEST_CASE("optional_services") {
    auto source = make_config();
    source["service"] = "inside";
    source["country"] = "CA";
    source["sampling"] = "20ms";
    source["feed_timeout"] = "50ms";
    source["gap_timeout"] = "30ms";
    source["receive_buffer"] = 4096;
    auto server = YAML::Load(R"(
address: "192.26.98.86:21100"
interface: "10.0.0.1:0"
sender: subscriber
timeout: 2s
)");
    auto snapshot = YAML::Load(R"(
address: "224.0.23.214:21005"
interface: "10.0.0.1"
timeout: 10s
)");
    snapshot["server"] = server;
    auto has_recovery = false;
    auto has_snapshot = false;
    SUBCASE("neither") {}
    SUBCASE("recovery") {
      has_recovery = true;
    }
    SUBCASE("snapshot") {
      has_snapshot = true;
    }
    SUBCASE("both") {
      has_recovery = true;
      has_snapshot = true;
    }
    if(has_recovery) {
      source["recovery"] = server;
    }
    if(has_snapshot) {
      source["snapshot"] = snapshot;
    }
    auto config = OtcLinkConfiguration::parse(source);
    REQUIRE(config.m_channel == 14);
    REQUIRE(config.m_country == Countries::CA);
    REQUIRE(config.m_sampling == milliseconds(20));
    REQUIRE(config.m_feed_timeout == milliseconds(50));
    REQUIRE(config.m_gap_timeout == milliseconds(30));
    REQUIRE(config.get_timer_interval() == milliseconds(30));
    REQUIRE(config.m_socket_options.m_receive_buffer_size == 4096);
    REQUIRE(config.m_recovery.has_value() == has_recovery);
    REQUIRE(config.m_snapshot.has_value() == has_snapshot);
    if(config.m_recovery) {
      REQUIRE(config.m_recovery->m_address == IpAddress("192.26.98.86", 21100));
      REQUIRE(config.m_recovery->m_interface == IpAddress("10.0.0.1", 0));
      REQUIRE(config.m_recovery->m_sender == "subscriber");
      REQUIRE(config.m_recovery->m_timeout == seconds(2));
    }
    if(config.m_snapshot) {
      auto& settings = *config.m_snapshot;
      REQUIRE(settings.m_feed.m_address == IpAddress("224.0.23.214", 21005));
      REQUIRE(settings.m_feed.m_interface == IpAddress("10.0.0.1", 0));
      REQUIRE(settings.m_timeout == seconds(10));
      REQUIRE(settings.m_server.m_address == IpAddress("192.26.98.86", 21100));
      REQUIRE(settings.m_server.m_interface == IpAddress("10.0.0.1", 0));
      REQUIRE(settings.m_server.m_sender == "subscriber");
      REQUIRE(settings.m_server.m_timeout == seconds(2));
    }
  }


  TEST_CASE("invalid_configuration") {
    auto source = make_config();
    SUBCASE("service") {
      for(auto value : {"", "global", "Book"}) {
        source["service"] = value;
        REQUIRE_THROWS_AS(
          OtcLinkConfiguration::parse(source), std::runtime_error);
      }
      source.remove("service");
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("country") {
      source["country"] = "unknown";
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("feeds") {
      for(auto value : {"[]", "{}", "null"}) {
        source["feeds"] = YAML::Load(value);
        REQUIRE_THROWS_AS(
          OtcLinkConfiguration::parse(source), std::runtime_error);
      }
      source.remove("feeds");
      REQUIRE_THROWS_AS(
        OtcLinkConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("duration") {
      for(auto key : {"sampling", "feed_timeout", "gap_timeout"}) {
        for(auto value : {"0s", "-1s", "infinity", "not-a-date-time"}) {
          source[key] = value;
          REQUIRE_THROWS_AS(
            OtcLinkConfiguration::parse(source), std::runtime_error);
        }
        source.remove(key);
      }
    }
    SUBCASE("receive_buffer") {
      for(auto value : {"0", "-1", "2147483648"}) {
        source["receive_buffer"] = value;
        REQUIRE_THROWS_AS(
          OtcLinkConfiguration::parse(source), std::runtime_error);
      }
    }
  }


  TEST_CASE("endpoints") {
    auto source = make_config()["feeds"][0];
    source["sender"] = "subscriber";
    for(auto value : {"", ":21000", "224.0.23.210", "224.0.23.210:0",
        "224.0.23.210:-1", "224.0.23.210:65536", "224.0.23.210:65537",
        "224.0.23.210:1x", "224.0.23.210:", " :21000"}) {
      source["address"] = value;
      REQUIRE_THROWS_AS(OtcLinkFeed::parse(source), std::runtime_error);
      REQUIRE_THROWS_AS(
        OtcLinkRecoveryConfiguration::parse(source), std::runtime_error);
    }
    source["address"] = "224.0.23.210:65535";
    REQUIRE(OtcLinkFeed::parse(source).m_address.get_port() == 65535);
    for(auto value : {"10.0.0.1", "10.0.0.1:0"}) {
      source["interface"] = value;
      REQUIRE(OtcLinkFeed::parse(source).m_interface ==
        IpAddress("10.0.0.1", 0));
    }
    for(auto value : {"", ":0", "10.0.0.1:-1", "10.0.0.1:65536"}) {
      source["interface"] = value;
      REQUIRE_THROWS_AS(OtcLinkFeed::parse(source), std::runtime_error);
    }
    source.remove("interface");
    REQUIRE_THROWS_AS(OtcLinkFeed::parse(source), std::runtime_error);
  }


  TEST_CASE("snapshot_and_recovery_settings") {
    auto server = YAML::Load(R"(
address: "192.26.98.86:21100"
interface: "10.0.0.1:0"
sender: subscriber
)");
    auto snapshot = YAML::Load(R"(
address: "224.0.23.210:21001"
interface: "10.0.0.1:21001"
)");
    snapshot["server"] = server;
    REQUIRE(OtcLinkRecoveryConfiguration::parse(server).m_timeout ==
      seconds(1));
    REQUIRE(OtcLinkSnapshotConfiguration::parse(snapshot).m_timeout ==
      seconds(30));
    for(auto value : {"0s", "-1s", "infinity", "not-a-date-time"}) {
      server["timeout"] = value;
      REQUIRE_THROWS_AS(
        OtcLinkRecoveryConfiguration::parse(server), std::runtime_error);
      server.remove("timeout");
      snapshot["timeout"] = value;
      REQUIRE_THROWS_AS(
        OtcLinkSnapshotConfiguration::parse(snapshot), std::runtime_error);
    }
    snapshot.remove("timeout");
    for(auto value : {"", "bad\x01sender"}) {
      server["sender"] = value;
      REQUIRE_THROWS_AS(
        OtcLinkRecoveryConfiguration::parse(server), std::runtime_error);
    }
    server.remove("sender");
    REQUIRE_THROWS_AS(
      OtcLinkRecoveryConfiguration::parse(server), std::runtime_error);
    snapshot.remove("server");
    REQUIRE_THROWS_AS(
      OtcLinkSnapshotConfiguration::parse(snapshot), std::runtime_error);
  }

}
