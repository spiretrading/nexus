#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpConfiguration.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  YAML::Node make_config() {
    return YAML::Load(R"(
feeds:
  - address: "233.102.209.224:60000"
    interface: "192.0.2.1"
recovery:
  address: "142.201.149.44:60020"
  interface: "192.0.2.2:0"
  delivery_address: "192.0.2.3:60050"
)");
  }
}

TEST_SUITE("TmxIpConfiguration") {
  TEST_CASE("mpid_mappings") {
    auto source = make_config();
    SUBCASE("absent") {
      REQUIRE(TmxIpConfiguration::parse(source).m_mpid_mappings.empty());
    }
    SUBCASE("configured") {
      source["mpid_mappings"] = YAML::Load(R"(
- source: 1
  name: ANON
- source: 79
  name: CIBC
- source: 69
  name: CITI
- source: 123
  name: CITI
)");
      auto config = TmxIpConfiguration::parse(source);
      REQUIRE(config.m_mpid_mappings.size() == 4);
      REQUIRE(config.m_mpid_mappings.at(1) == "ANON");
      REQUIRE(config.m_mpid_mappings.at(79) == "CIBC");
      REQUIRE(config.m_mpid_mappings.at(69) == "CITI");
      REQUIRE(config.m_mpid_mappings.at(123) == "CITI");
    }
    SUBCASE("invalid") {
      for(auto value : {"{}", "[{source: -1, name: ANON}]",
          "[{source: 1000, name: ANON}]", "[{source: 1, name: ' '}]",
          "[{source: 1}]", "[{name: ANON}]",
          "[{source: 1, name: ANON}, {source: 1, name: CIBC}]"}) {
        source["mpid_mappings"] = YAML::Load(value);
        REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source),
          std::runtime_error);
      }
    }
  }

  TEST_CASE("time_zone") {
    auto source = make_config();
    auto offset = hours(-5);
    SUBCASE("default") {}
    SUBCASE("venue") {
      source["venue"] = "ASX";
      offset = hours(10);
    }
    SUBCASE("configured") {
      source["time_zone"] = "Asia/Tokyo";
      offset = hours(9);
    }
    SUBCASE("configured_precedence") {
      source["venue"] = "ASX";
      source["time_zone"] = "Asia/Tokyo";
      offset = hours(9);
    }
    auto config = TmxIpConfiguration::parse(source);
    REQUIRE(config.m_time_zone);
    REQUIRE(config.m_time_zone->base_utc_offset() == offset);
  }

  TEST_CASE("invalid_time_zone") {
    auto source = make_config();
    for(auto value : {"", "Unknown/Zone"}) {
      source["time_zone"] = value;
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }

  TEST_CASE("rollover_time") {
    auto source = make_config();
    auto value = std::string("00:00:00");
    auto expected = time_duration();
    SUBCASE("midnight") {}
    SUBCASE("fractional") {
      value = "23:59:59.999";
      expected = duration_from_string(value);
    }
    source["rollover_time"] = value;
    REQUIRE(TmxIpConfiguration::parse(source).m_rollover_time == expected);
  }

  TEST_CASE("invalid_rollover_time") {
    auto source = make_config();
    for(auto value :
        {"-00:00:01", "24:00:00", "infinity", "not-a-date-time", ":",
          "00::30", "00:30:", "00:30:00:01", "00:60:00", "00:00:60",
          "00:30:00.", "00:30:00.extra"}) {
      source["rollover_time"] = value;
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }

  TEST_CASE("parse") {
    auto source = make_config();
    SUBCASE("defaults") {
      auto config = TmxIpConfiguration::parse(source);
      REQUIRE(config.m_country == Countries::CA);
      REQUIRE(config.m_sampling == milliseconds(100));
      REQUIRE(config.m_rollover_time == minutes(30));
      REQUIRE(!config.m_is_logging_messages);
      REQUIRE(config.m_feeds.size() == 1);
      REQUIRE(config.m_recovery.has_value());
      REQUIRE(config.m_feed_timeout == seconds(1));
      REQUIRE(config.m_gap_timeout == seconds(1));
      REQUIRE(config.m_feeds.front().m_address.get_host() == "233.102.209.224");
      REQUIRE(config.m_feeds.front().m_address.get_port() == 60000);
      REQUIRE(config.m_feeds.front().m_interface.get_host() == "192.0.2.1");
      REQUIRE(config.m_feeds.front().m_interface.get_port() == 0);
      REQUIRE(config.m_recovery->m_address.get_host() == "142.201.149.44");
      REQUIRE(config.m_recovery->m_address.get_port() == 60020);
      REQUIRE(config.m_recovery->m_interface.get_host() == "192.0.2.2");
      REQUIRE(config.m_recovery->m_interface.get_port() == 0);
      REQUIRE(config.m_recovery->m_delivery_address.get_host() == "192.0.2.3");
      REQUIRE(config.m_recovery->m_delivery_address.get_port() == 60050);
      REQUIRE(config.m_recovery->m_timeout == seconds(30));
      REQUIRE(config.m_retry_interval == seconds(1));
      REQUIRE(
        config.m_socket_options.m_receive_buffer_size == 128 * 1024 * 1024);
      REQUIRE(!config.m_socket_options.m_enable_loopback);
      REQUIRE(config.m_socket_options.m_max_datagram_size ==
        std::numeric_limits<std::uint16_t>::max());
    }
    SUBCASE("overrides") {
      source["country"] = "AU";
      source["rollover_time"] = "06:00:00";
      source["sampling"] = "250ms";
      source["enable_logging"] = true;
      source["receive_buffer"] = 1048576;
      source["retry_interval"] = "250ms";
      source["feed_timeout"] = "2s";
      source["gap_timeout"] = "10s";
      source["recovery"]["timeout"] = "15s";
      auto config = TmxIpConfiguration::parse(source);
      REQUIRE(config.m_country == Countries::AU);
      REQUIRE(config.m_rollover_time == hours(6));
      REQUIRE(config.m_sampling == milliseconds(250));
      REQUIRE(config.m_is_logging_messages);
      REQUIRE(config.m_socket_options.m_receive_buffer_size == 1048576);
      REQUIRE(config.m_retry_interval == milliseconds(250));
      REQUIRE(config.m_feed_timeout == seconds(2));
      REQUIRE(config.m_gap_timeout == seconds(10));
      REQUIRE(config.m_recovery->m_timeout == seconds(15));
    }
  }

  TEST_CASE("venue") {
    auto source = make_config();
    SUBCASE("configured") {
      source["venue"] = "TSX";
      REQUIRE(TmxIpConfiguration::parse(source).m_venue == Venues::TSX);
      source["venue"] = "NEON";
      REQUIRE(TmxIpConfiguration::parse(source).m_venue == Venues::NEON);
    }
    SUBCASE("consolidated") {
      REQUIRE_FALSE(TmxIpConfiguration::parse(source).m_venue);
    }
    SUBCASE("invalid") {
      for(auto value : {"", "UNKNOWN"}) {
        source["venue"] = value;
        REQUIRE_THROWS_AS(
          TmxIpConfiguration::parse(source), std::runtime_error);
      }
    }
  }

  TEST_CASE("required_fields") {
    auto source = make_config();
    SUBCASE("sections") {
      for(auto name : {"feeds"}) {
        source = make_config();
        source.remove(name);
        REQUIRE_THROWS_AS(
          TmxIpConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("addresses") {
      for(auto section : {"feeds", "recovery"}) {
        for(auto name : {"address", "interface"}) {
          source = make_config();
          auto node = source[section];
          if(std::string_view(section) == "feeds") {
            node.reset(source[section][0]);
          }
          node.remove(name);
          REQUIRE_THROWS_AS(
            TmxIpConfiguration::parse(source), std::runtime_error);
        }
      }
      source = make_config();
      source["recovery"].remove("delivery_address");
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }


  TEST_CASE("invalid_address") {
    auto source = make_config();
    auto section = source["feeds"][0];
    auto key = std::string("address");
    SUBCASE("feed") {}
    SUBCASE("recovery_server") {
      section.reset(source["recovery"]);
    }
    SUBCASE("recovery_delivery") {
      section.reset(source["recovery"]);
      key = "delivery_address";
    }
    for(auto value : {"192.0.2.1", "192.0.2.1:0", ":60000", "",
        "192.0.2.1:65536", "192.0.2.1:-1"}) {
      CAPTURE(std::string(value));
      section[key] = value;
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }


  TEST_CASE("invalid_duration") {
    auto source = make_config();
    auto section = source;
    auto key = std::string("retry_interval");
    SUBCASE("retry_interval") {}
    SUBCASE("sampling") {
      key = "sampling";
    }
    SUBCASE("feed_timeout") {
      key = "feed_timeout";
    }
    SUBCASE("gap_timeout") {
      key = "gap_timeout";
    }
    SUBCASE("recovery_timeout") {
      section.reset(source["recovery"]);
      key = "timeout";
    }
    for(auto value :
        {"0s", "-1s", "infinity", "-infinity", "not-a-date-time"}) {
      CAPTURE(std::string(value));
      section[key] = value;
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }


  TEST_CASE("invalid_options") {
    auto source = make_config();
    SUBCASE("receive_buffer") {
      for(auto value : {"0", "-1", "2147483648", "abc"}) {
        source["receive_buffer"] = value;
        REQUIRE_THROWS_AS(
          TmxIpConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("interfaces") {
      for(auto name : {"feeds", "recovery"}) {
        for(auto value : {"", ":0", "192.0.2.1:-1"}) {
          source = make_config();
          auto node = source[name];
          if(std::string_view(name) == "feeds") {
            node.reset(source[name][0]);
          }
          node["interface"] = value;
          REQUIRE_THROWS_AS(
            TmxIpConfiguration::parse(source), std::runtime_error);
        }
      }
    }
    SUBCASE("country") {
      for(auto country : {"", "ZZ", "INVALID"}) {
        source["country"] = country;
        REQUIRE_THROWS_AS(
          TmxIpConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("logging") {
      source["enable_logging"] = "invalid";
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }


  TEST_CASE("optional_recovery") {
    auto source = make_config();
    source.remove("recovery");
    auto config = TmxIpConfiguration::parse(source);
    REQUIRE(!config.m_recovery);
  }

  TEST_CASE("feeds") {
    auto source = make_config();
    SUBCASE("multiple") {
      source["feeds"].push_back(YAML::Load(R"(
address: "233.102.209.96:60001"
interface: "192.0.2.4"
)"));
      auto config = TmxIpConfiguration::parse(source);
      REQUIRE(config.m_feeds.size() == 2);
      REQUIRE(config.m_feeds[0].m_address.get_host() == "233.102.209.224");
      REQUIRE(config.m_feeds[1].m_address.get_host() == "233.102.209.96");
      REQUIRE(config.m_feeds[1].m_address.get_port() == 60001);
      REQUIRE(config.m_feeds[1].m_interface.get_host() == "192.0.2.4");
    }
    SUBCASE("empty") {
      source["feeds"] = YAML::Node(YAML::NodeType::Sequence);
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("mapping") {
      source["feeds"] = YAML::Clone(source["feeds"][0]);
      REQUIRE_THROWS_AS(TmxIpConfiguration::parse(source), std::runtime_error);
    }
  }

}
