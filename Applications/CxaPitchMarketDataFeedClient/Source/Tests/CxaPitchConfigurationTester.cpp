#include <stdexcept>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchConfiguration.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  YAML::Node make_config() {
    return YAML::Load(R"(
unit: 1
venue: ASX
disseminating_venue: CXA
feeds:
  - name: A
    address: "233.218.133.80:30501"
    interface: "10.0.0.1:30501"
)");
  }

  YAML::Node make_session() {
    return YAML::Load(R"(
address: "10.1.0.1:30601"
session_sub_id: "0001"
username: FIRM
password: ABCD00
)");
  }
}

TEST_SUITE("CxaPitchConfiguration") {
  TEST_CASE("parse_every_field") {
    auto source = make_config();
    source["mpid"] = "XCXA";
    source["enable_logging"] = true;
    source["feed_timeout"] = "5s";
    source["gap_timeout"] = "2s";
    source["feeds"][0]["gap_address"] = "233.218.133.81:30501";
    source["feeds"].push_back(YAML::Load(R"(
name: B
address: "233.218.133.96:30501"
gap_address: "233.218.133.97:30501"
interface: "10.0.0.2:30501"
)"));
    source["retransmission"] = YAML::Load(R"(
address: "10.1.0.1:30601"
session_sub_id: "0001"
username: FIRM
password: ABCD00
)");
    source["spin"] = YAML::Load(R"(
address: "10.1.0.2:30701"
session_sub_id: "0002"
username: FIRM
password: ABCD01
)");
    auto config = CxaPitchConfiguration::parse(source);
    REQUIRE(config.m_is_logging_messages);
    REQUIRE(config.m_unit == 1);
    REQUIRE(config.m_country == Countries::AU);
    REQUIRE(config.m_primary_venue == Venues::ASX);
    REQUIRE(config.m_disseminating_venue == Venues::CXA);
    REQUIRE(config.m_mpid == "XCXA");
    REQUIRE(config.m_feed_timeout == duration_from_string("00:00:05"));
    REQUIRE(config.m_gap_timeout == duration_from_string("00:00:02"));
    REQUIRE(config.m_feeds.size() == 2);
    REQUIRE(config.m_feeds[0].m_name == "A");
    REQUIRE(config.m_feeds[0].m_address.get_host() == "233.218.133.80");
    REQUIRE(config.m_feeds[0].m_address.get_port() == 30501);
    REQUIRE(config.m_feeds[0].m_gap_address.has_value());
    REQUIRE(config.m_feeds[0].m_gap_address->get_host() == "233.218.133.81");
    REQUIRE(config.m_feeds[0].m_gap_address->get_port() == 30501);
    REQUIRE(config.m_feeds[0].m_interface.get_host() == "10.0.0.1");
    REQUIRE(config.m_feeds[0].m_interface.get_port() == 30501);
    REQUIRE(config.m_feeds[1].m_name == "B");
    REQUIRE(config.m_feeds[1].m_address.get_host() == "233.218.133.96");
    REQUIRE(config.m_feeds[1].m_address.get_port() == 30501);
    REQUIRE(config.m_feeds[1].m_gap_address.has_value());
    REQUIRE(config.m_feeds[1].m_gap_address->get_host() == "233.218.133.97");
    REQUIRE(config.m_feeds[1].m_gap_address->get_port() == 30501);
    REQUIRE(config.m_feeds[1].m_interface.get_host() == "10.0.0.2");
    REQUIRE(config.m_feeds[1].m_interface.get_port() == 30501);
    REQUIRE(config.m_retransmission.has_value());
    REQUIRE(config.m_retransmission->m_address.get_host() == "10.1.0.1");
    REQUIRE(config.m_retransmission->m_address.get_port() == 30601);
    REQUIRE(config.m_retransmission->m_session_sub_id == "0001");
    REQUIRE(config.m_retransmission->m_username == "FIRM");
    REQUIRE(config.m_retransmission->m_password == "ABCD00");
    REQUIRE(config.m_spin.has_value());
    REQUIRE(config.m_spin->m_address.get_host() == "10.1.0.2");
    REQUIRE(config.m_spin->m_address.get_port() == 30701);
    REQUIRE(config.m_spin->m_session_sub_id == "0002");
    REQUIRE(config.m_spin->m_username == "FIRM");
    REQUIRE(config.m_spin->m_password == "ABCD01");
  }

  TEST_CASE("parse_defaults") {
    auto source = make_config();
    source["unit"] = 2;
    source["feeds"][0]["address"] = "233.218.133.82:30502";
    source["feeds"][0]["interface"] = "10.0.0.1:30502";
    auto config = CxaPitchConfiguration::parse(source);
    REQUIRE(!config.m_is_logging_messages);
    REQUIRE(config.m_unit == 2);
    REQUIRE(config.m_mpid == "CXA");
    REQUIRE(config.m_feed_timeout == duration_from_string("00:00:03"));
    REQUIRE(config.m_gap_timeout == duration_from_string("00:00:05"));
    REQUIRE(config.m_feeds.size() == 1);
    REQUIRE(config.m_feeds[0].m_interface.get_host() == "10.0.0.1");
    REQUIRE(!config.m_feeds[0].m_gap_address);
    REQUIRE(!config.m_retransmission);
    REQUIRE(!config.m_spin);
  }

  TEST_CASE("parse_timeouts") {
    auto source = make_config();
    SUBCASE("feed") {
      for(auto value : {"0s", "-1us", "infinity", "+infinity", "-infinity"}) {
        CAPTURE(std::string_view(value));
        source["feed_timeout"] = value;
        REQUIRE_THROWS_AS(
          CxaPitchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("gap") {
      for(auto value : {"-1us", "infinity", "+infinity", "-infinity"}) {
        CAPTURE(std::string_view(value));
        source["gap_timeout"] = value;
        REQUIRE_THROWS_AS(
          CxaPitchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("bounds") {
      source["feed_timeout"] = "1us";
      source["gap_timeout"] = "0s";
      auto config = CxaPitchConfiguration::parse(source);
      REQUIRE(config.m_feed_timeout == duration_from_string("00:00:00.000001"));
      REQUIRE(config.m_gap_timeout == duration_from_string("00:00:00"));
    }
  }

  TEST_CASE("parse_empty_feed_list") {
    auto source = make_config();
    source["feeds"] = YAML::Node(YAML::NodeType::Sequence);
    REQUIRE_THROWS_AS(
      CxaPitchConfiguration::parse(source), std::runtime_error);
  }

  TEST_CASE("parse_missing_unit") {
    auto source = make_config();
    source.remove("unit");
    REQUIRE_THROWS_AS(
      CxaPitchConfiguration::parse(source), std::runtime_error);
  }

  TEST_CASE("parse_missing_fields") {
    SUBCASE("feed") {
      for(auto name : {"name", "address", "interface"}) {
        CAPTURE(std::string_view(name));
        auto source = make_config();
        source["feeds"][0].remove(name);
        REQUIRE_THROWS_AS(
          CxaPitchConfiguration::parse(source), std::runtime_error);
      }
    }
    SUBCASE("session") {
      for(auto name : {"retransmission", "spin"}) {
        CAPTURE(std::string_view(name));
        for(auto field : {"address", "session_sub_id", "username",
            "password"}) {
          CAPTURE(std::string_view(field));
          auto source = make_config();
          source["feeds"][0]["gap_address"] = "233.218.133.81:30501";
          source[name] = make_session();
          source[name].remove(field);
          REQUIRE_THROWS_AS(
            CxaPitchConfiguration::parse(source), std::runtime_error);
        }
      }
    }
  }

  TEST_CASE("parse_retransmission") {
    auto source = make_config();
    source["feeds"].push_back(YAML::Load(R"(
name: B
address: "233.218.133.96:30501"
interface: "10.0.0.2:30501"
)"));
    source["retransmission"] = make_session();
    SUBCASE("no_gap_address") {
      REQUIRE_THROWS_AS(
        CxaPitchConfiguration::parse(source), std::runtime_error);
    }
    SUBCASE("single_gap_address") {
      for(auto index : {0, 1}) {
        CAPTURE(index);
        source["feeds"][index]["gap_address"] = "233.218.133.81:30501";
        auto config = CxaPitchConfiguration::parse(source);
        REQUIRE(config.m_retransmission.has_value());
        REQUIRE(!config.m_spin);
        REQUIRE(config.m_feeds.size() == 2);
        REQUIRE(config.m_feeds[index].m_gap_address.has_value());
        REQUIRE(config.m_feeds[index].m_gap_address->get_host() ==
          "233.218.133.81");
        REQUIRE(config.m_feeds[index].m_gap_address->get_port() == 30501);
        REQUIRE(!config.m_feeds[1 - index].m_gap_address);
        source["feeds"][index].remove("gap_address");
      }
    }
  }

  TEST_CASE("parse_spin") {
    auto source = make_config();
    source["spin"] = make_session();
    auto config = CxaPitchConfiguration::parse(source);
    REQUIRE(config.m_spin.has_value());
    REQUIRE(!config.m_retransmission);
    REQUIRE(config.m_feeds.size() == 1);
    REQUIRE(!config.m_feeds[0].m_gap_address);
  }

  TEST_CASE("parse_unit") {
    auto source = make_config();
    SUBCASE("bounds") {
      for(auto unit : {1, 255}) {
        source["unit"] = unit;
        auto config = CxaPitchConfiguration::parse(source);
        REQUIRE(config.m_unit == unit);
      }
    }
    SUBCASE("invalid") {
      for(auto unit : {"1.5", "1junk", ""}) {
        CAPTURE(std::string_view(unit));
        source["unit"] = unit;
        REQUIRE_THROWS_AS(
          CxaPitchConfiguration::parse(source), std::runtime_error);
      }
    }
  }

  TEST_CASE("parse_unit_out_of_range") {
    for(auto unit : {0, 256}) {
      auto source = make_config();
      source["unit"] = unit;
      REQUIRE_THROWS_AS(
        CxaPitchConfiguration::parse(source), std::runtime_error);
    }
  }

  TEST_CASE("parse_unknown_venue") {
    for(auto name : {"venue", "disseminating_venue"}) {
      CAPTURE(std::string_view(name));
      auto source = make_config();
      source[name] = "NOPE";
      REQUIRE_THROWS_AS(
        CxaPitchConfiguration::parse(source), std::runtime_error);
    }
  }
}
