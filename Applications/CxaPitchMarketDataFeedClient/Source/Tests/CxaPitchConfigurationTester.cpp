#include <stdexcept>
#include <string>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchConfiguration.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"

using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("CxaPitchConfiguration") {
  TEST_CASE("parse_every_field") {
    auto source = std::string(R"(
unit: 1
venue: ASX
disseminating_venue: CXA
mpid: XCXA
enable_logging: true
liveness: 5s
interface: "10.0.0.1"
feeds:
  - name: A
    address: "233.218.133.80:30501"
    gap_address: "233.218.133.81:30501"
  - name: B
    address: "233.218.133.96:30501"
    gap_address: "233.218.133.97:30501"
    interface: "10.0.0.2"
retransmission:
  address: "10.1.0.1:30601"
  session_sub_id: "0001"
  username: FIRM
  password: ABCD00
spin:
  address: "10.1.0.2:30701"
  session_sub_id: "0002"
  username: FIRM
  password: ABCD01
)");
    auto config = CxaPitchConfiguration::parse(YAML::Load(source));
    REQUIRE(config.m_is_logging_messages);
    REQUIRE(config.m_unit == 1);
    REQUIRE(config.m_country == Countries::AU);
    REQUIRE(config.m_primary_venue == Venues::ASX);
    REQUIRE(config.m_disseminating_venue == Venues::CXA);
    REQUIRE(config.m_mpid == "XCXA");
    REQUIRE(config.m_liveness == seconds(5));
    REQUIRE(config.m_feeds.size() == 2);
    REQUIRE(config.m_feeds[0].m_name == "A");
    REQUIRE(config.m_feeds[0].m_address.get_host() == "233.218.133.80");
    REQUIRE(config.m_feeds[0].m_address.get_port() == 30501);
    REQUIRE(config.m_feeds[0].m_gap_address.get_host() == "233.218.133.81");
    REQUIRE(config.m_feeds[0].m_interface.get_host() == "10.0.0.1");
    REQUIRE(config.m_feeds[1].m_name == "B");
    REQUIRE(config.m_feeds[1].m_address.get_host() == "233.218.133.96");
    REQUIRE(config.m_feeds[1].m_interface.get_host() == "10.0.0.2");
    REQUIRE(config.m_retransmission->m_address.get_host() == "10.1.0.1");
    REQUIRE(config.m_retransmission->m_address.get_port() == 30601);
    REQUIRE(config.m_retransmission->m_session_sub_id == "0001");
    REQUIRE(config.m_retransmission->m_username == "FIRM");
    REQUIRE(config.m_retransmission->m_password == "ABCD00");
    REQUIRE(config.m_spin->m_address.get_port() == 30701);
    REQUIRE(config.m_spin->m_session_sub_id == "0002");
    REQUIRE(config.m_spin->m_password == "ABCD01");
  }

  TEST_CASE("parse_defaults") {
    auto source = std::string(R"(
unit: 2
venue: ASX
disseminating_venue: CXA
interface: "10.0.0.1"
feeds:
  - name: A
    address: "233.218.133.82:30502"
)");
    auto config = CxaPitchConfiguration::parse(YAML::Load(source));
    REQUIRE(!config.m_is_logging_messages);
    REQUIRE(config.m_unit == 2);
    REQUIRE(config.m_mpid == "CXA");
    REQUIRE(config.m_liveness == seconds(3));
    REQUIRE(config.m_feeds.size() == 1);
    REQUIRE(config.m_feeds[0].m_interface.get_host() == "10.0.0.1");
    REQUIRE(config.m_feeds[0].m_gap_address.get_host().empty());
    REQUIRE(!config.m_retransmission);
    REQUIRE(!config.m_spin);
  }

  TEST_CASE("parse_rejects_an_empty_feed_list") {
    auto source = std::string(R"(
unit: 1
venue: ASX
disseminating_venue: CXA
interface: "10.0.0.1"
feeds: []
)");
    REQUIRE_THROWS_AS(
      CxaPitchConfiguration::parse(YAML::Load(source)), std::runtime_error);
  }

  TEST_CASE("parse_rejects_a_missing_unit") {
    auto source = std::string(R"(
venue: ASX
disseminating_venue: CXA
interface: "10.0.0.1"
feeds:
  - name: A
    address: "233.218.133.80:30501"
)");
    REQUIRE_THROWS_AS(
      CxaPitchConfiguration::parse(YAML::Load(source)), std::runtime_error);
  }

  TEST_CASE("parse_rejects_an_unknown_venue") {
    auto source = std::string(R"(
unit: 1
venue: NOPE
disseminating_venue: CXA
interface: "10.0.0.1"
feeds:
  - name: A
    address: "233.218.133.80:30501"
)");
    REQUIRE_THROWS_AS(
      CxaPitchConfiguration::parse(YAML::Load(source)), std::runtime_error);
  }
}
