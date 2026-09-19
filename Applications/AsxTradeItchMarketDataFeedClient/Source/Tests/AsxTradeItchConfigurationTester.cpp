#include <boost/date_time/posix_time/time_parsers.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchConfiguration.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  YAML::Node make_config() {
    return YAML::Load(R"(
partition: 1
sampling: 100ms
venue: ASX
disseminating_venue: ASX
feeds:
  - name: A
    address: "233.71.185.129:21001"
    interface: "10.0.0.1:21001"
rewind:
  address: "203.6.253.126:24001"
  interface: "10.0.0.1:0"
)");
  }

  YAML::Node make_glimpse() {
    return YAML::Load(R"(
address: "203.6.253.126:21801"
interface: "10.0.0.1:0"
username: TEST01
password: TESTSECRET
)");
  }
}

TEST_SUITE("AsxTradeItchConfiguration") {
  TEST_CASE("parse") {
    auto source = make_config();
    SUBCASE("defaults") {
      auto config = AsxTradeItchConfiguration::parse(source);
      REQUIRE(!config.m_is_logging_messages);
      REQUIRE(config.m_partition == 1);
      REQUIRE(config.m_country == Countries::AU);
      REQUIRE(config.m_primary_venue == Venues::ASX);
      REQUIRE(config.m_disseminating_venue == Venues::ASX);
      REQUIRE(config.m_mpid == "ASX");
      REQUIRE(config.m_sampling == milliseconds(100));
      REQUIRE(config.m_feed_timeout == seconds(3));
      REQUIRE(config.m_request_timeout == seconds(1));
      REQUIRE(config.get_timer_interval() == milliseconds(100));
      REQUIRE(config.m_feeds.size() == 1);
      REQUIRE(config.m_feeds[0].m_name == "A");
      REQUIRE(config.m_feeds[0].m_address.get_host() == "233.71.185.129");
      REQUIRE(config.m_feeds[0].m_address.get_port() == 21001);
      REQUIRE(config.m_feeds[0].m_interface.get_host() == "10.0.0.1");
      REQUIRE(config.m_feeds[0].m_interface.get_port() == 21001);
      REQUIRE(config.m_rewind.m_address.get_host() == "203.6.253.126");
      REQUIRE(config.m_rewind.m_address.get_port() == 24001);
      REQUIRE(config.m_rewind.m_interface.get_host() == "10.0.0.1");
      REQUIRE(config.m_rewind.m_interface.get_port() == 0);
      REQUIRE(!config.m_glimpse);
      REQUIRE(
        config.m_socket_options.m_receive_buffer_size == 128 * 1024 * 1024);
      REQUIRE(config.m_socket_options.m_max_datagram_size ==
        std::numeric_limits<std::uint16_t>::max());
    }
    SUBCASE("overrides") {
      source["partition"] = 4;
      source["mpid"] = "XASX";
      source["enable_logging"] = true;
      source["sampling"] = "250ms";
      source["feed_timeout"] = "50ms";
      source["request_timeout"] = "20ms";
      source["receive_buffer"] = 1048576;
      source["mtu"] = 9000;
      source["glimpse"] = make_glimpse();
      source["feeds"].push_back(YAML::Load(R"(
name: B
address: "233.71.185.145:21101"
interface: "10.0.0.2:21101"
)"));
      auto config = AsxTradeItchConfiguration::parse(source);
      REQUIRE(config.m_partition == 4);
      REQUIRE(config.m_is_logging_messages);
      REQUIRE(config.m_mpid == "XASX");
      REQUIRE(config.m_sampling == milliseconds(250));
      REQUIRE(config.m_feed_timeout == milliseconds(50));
      REQUIRE(config.m_request_timeout == milliseconds(20));
      REQUIRE(config.get_timer_interval() == milliseconds(20));
      REQUIRE(config.m_socket_options.m_receive_buffer_size == 1048576);
      REQUIRE(config.m_socket_options.m_max_datagram_size == 9000);
      REQUIRE(config.m_feeds.size() == 2);
      REQUIRE(config.m_feeds[1].m_name == "B");
      REQUIRE(config.m_feeds[1].m_address.get_host() == "233.71.185.145");
      REQUIRE(config.m_feeds[1].m_address.get_port() == 21101);
      REQUIRE(config.m_feeds[1].m_interface.get_host() == "10.0.0.2");
      REQUIRE(config.m_glimpse.has_value());
      REQUIRE(config.m_glimpse->m_address.get_host() == "203.6.253.126");
      REQUIRE(config.m_glimpse->m_address.get_port() == 21801);
      REQUIRE(config.m_glimpse->m_interface.get_host() == "10.0.0.1");
      REQUIRE(config.m_glimpse->m_interface.get_port() == 0);
      REQUIRE(config.m_glimpse->m_username == "TEST01");
      REQUIRE(config.m_glimpse->m_password == "TESTSECRET");
      config.m_request_timeout = seconds(1);
      REQUIRE(config.get_timer_interval() == config.m_feed_timeout);
    }
  }

  TEST_CASE("invalid_configuration") {
    auto source = make_config();
    SUBCASE("partition") {
      for(auto value : {"0", "5", "257", "-1", "abc"}) {
        source["partition"] = value;
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("duration") {
      for(auto name : {"sampling", "feed_timeout", "request_timeout"}) {
        for(auto value : {"0s", "-1s", "infinity", "not-a-date-time"}) {
          source = make_config();
          source[name] = value;
          REQUIRE_THROWS_AS(
            AsxTradeItchConfiguration::parse(source), std::runtime_error);
        }
      }
    }
    SUBCASE("port") {
      for(auto name : {"feed", "rewind", "glimpse"}) {
        for(auto address : {"10.0.0.1", "10.0.0.1:0"}) {
          source = make_config();
          if(std::string_view(name) == "feed") {
            source["feeds"][0]["address"] = address;
          } else if(std::string_view(name) == "rewind") {
            source["rewind"]["address"] = address;
          } else {
            source["glimpse"] = make_glimpse();
            source["glimpse"]["address"] = address;
          }
          REQUIRE_THROWS_AS(
            AsxTradeItchConfiguration::parse(source), std::runtime_error);
        }
      }
    }
    SUBCASE("feeds") {
      for(auto value : {"[]", "{}", "null", "A"}) {
        source["feeds"] = YAML::Load(value);
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("missing_field") {
      for(auto name : {"partition", "sampling", "venue",
          "disseminating_venue", "feeds", "rewind"}) {
        source = make_config();
        source.remove(name);
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("venue") {
      for(auto name : {"venue", "disseminating_venue"}) {
        source = make_config();
        source[name] = "UNKNOWN";
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("socket_options") {
      for(auto value : {"0", "-1", "2147483648"}) {
        source["receive_buffer"] = value;
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
      source = make_config();
      for(auto value : {"0", "1499", "65536", "-1"}) {
        source["mtu"] = value;
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("glimpse") {
      for(auto name : {"address", "interface", "username", "password"}) {
        source["glimpse"] = make_glimpse();
        source["glimpse"].remove(name);
        REQUIRE_THROWS_AS(
          AsxTradeItchConfiguration::parse(source), std::runtime_error);
      }
      for(auto name : {"username", "password"}) {
        for(auto value : {"", "TOO_LONG_FOR_EITHER_FIELD"}) {
          source["glimpse"] = make_glimpse();
          source["glimpse"][name] = value;
          REQUIRE_THROWS_AS(
            AsxTradeItchConfiguration::parse(source), std::runtime_error);
        }
      }
    }
  }
}
