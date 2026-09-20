#include <ostream>
#include <string>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMessageBuilder.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("TmxIpMessageBuilder") {
  TEST_CASE("standalone") {
    auto builder = TmxIpMessageBuilder();
    auto source = std::string("\x02" "0036000000001CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d\x03");
    auto message = builder.add(TmxIpPacket::parse(source));
    REQUIRE(message.has_value());
    source.assign(source.size(), 'X');
    auto view = std::string_view(message->get_data(), message->get_size());
    REQUIRE(view == "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
    auto next = builder.add(TmxIpPacket::parse(
      "\x02" "0036000000002CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=XYZ\x1d\x03"));
    REQUIRE(next.has_value());
    builder.reset();
    auto parsed = StampMessage::parse(view);
    auto symbol = parsed.m_business_content.find(55);
    REQUIRE(symbol.has_value());
    REQUIRE(symbol->m_value == "ABX");
  }

  TEST_CASE("fragments") {
    auto builder = TmxIpMessageBuilder();
    auto source = std::string(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03");
    REQUIRE(!builder.add(TmxIpPacket::parse(source)));
    source.assign(source.size(), 'X');
    auto last = TmxIpPacket::parse(
      "\x02" "0031000000002CDF02  T \x1c\x1e" "55=ABX\x1d\x03");
    SUBCASE("two_packets") {}
    SUBCASE("three_packets") {
      REQUIRE(!builder.add(TmxIpPacket::parse(
        "\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03")));
      last = TmxIpPacket::parse(
        "\x02" "0024000000003CDF02  T X\x1d\x03");
    }
    auto message = builder.add(last);
    REQUIRE(message.has_value());
    auto view = std::string_view(message->get_data(), message->get_size());
    REQUIRE(view == "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
    REQUIRE(!builder.add(last));
    builder.reset();
    REQUIRE(view == "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }

  TEST_CASE("interrupted_message") {
    auto builder = TmxIpMessageBuilder();
    auto first = TmxIpPacket::parse(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03");
    auto middle = TmxIpPacket::parse(
      "\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03");
    auto last = TmxIpPacket::parse(
      "\x02" "0024000000003CDF02  T X\x1d\x03");
    REQUIRE(!builder.add(first));
    SUBCASE("missing_packet") {
      middle.m_header.m_sequence = 3;
    }
    SUBCASE("duplicate_packet") {
      middle.m_header.m_sequence = 1;
    }
    SUBCASE("service") {
      middle.m_header.m_service = "CB1";
    }
    SUBCASE("exchange") {
      middle.m_header.m_exchange = 'V';
    }
    SUBCASE("retransmission") {
      middle.m_header.m_retransmission = '1';
    }
    REQUIRE(!builder.add(middle));
    REQUIRE(!builder.add(last));
    REQUIRE(!builder.add(first));
    middle = TmxIpPacket::parse(
      "\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03");
    REQUIRE(!builder.add(middle));
    auto message = builder.add(last);
    REQUIRE(message.has_value());
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }

  TEST_CASE("heartbeat") {
    auto builder = TmxIpMessageBuilder();
    auto heartbeat = TmxIpPacket::parse("\x02" "0022         CDF 0V T \x03");
    REQUIRE(!builder.add(heartbeat));
    REQUIRE(!builder.add(TmxIpPacket::parse(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03")));
    REQUIRE(!builder.add(heartbeat));
    auto message = builder.add(TmxIpPacket::parse(
      "\x02" "0031000000002CDF02  T \x1c\x1e" "55=ABX\x1d\x03"));
    REQUIRE(message.has_value());
    REQUIRE(!builder.add(heartbeat));
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }

  TEST_CASE("replacement_message") {
    auto builder = TmxIpMessageBuilder();
    REQUIRE(!builder.add(TmxIpPacket::parse(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03")));
    auto message = boost::optional<SharedBuffer>();
    SUBCASE("first") {
      REQUIRE(!builder.add(TmxIpPacket::parse(
        "\x02" "0027000000010CB111  Q \x01\x1e" "1=J\x03")));
      message = builder.add(TmxIpPacket::parse(
        "\x02" "0031000000011CB112  Q \x1c\x1e" "55=XYZ\x1d\x03"));
    }
    SUBCASE("standalone") {
      message = builder.add(TmxIpPacket::parse(
        "\x02" "0036000000010CB110  Q "
        "\x01\x1e" "1=J\x1c\x1e" "55=XYZ\x1d\x03"));
    }
    REQUIRE(message.has_value());
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=J\x1c\x1e" "55=XYZ\x1d");
    REQUIRE(!builder.add(TmxIpPacket::parse(
      "\x02" "0031000000002CDF02  T \x1c\x1e" "55=ABX\x1d\x03")));
  }

  TEST_CASE("sequence_wrap") {
    auto builder = TmxIpMessageBuilder();
    REQUIRE(!builder.add(TmxIpPacket::parse(
      "\x02" "0027999999999CDF01  T \x01\x1e" "1=H\x03")));
    REQUIRE(!builder.add(TmxIpPacket::parse(
      "\x02" "0029000000001CDF03  T \x1c\x1e" "55=AB\x03")));
    auto message = builder.add(TmxIpPacket::parse(
      "\x02" "0024000000002CDF02  T X\x1d\x03"));
    REQUIRE(message.has_value());
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }

  TEST_CASE("reset") {
    auto builder = TmxIpMessageBuilder();
    auto first = TmxIpPacket::parse(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03");
    auto middle = TmxIpPacket::parse(
      "\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03");
    auto last = TmxIpPacket::parse(
      "\x02" "0024000000003CDF02  T X\x1d\x03");
    SUBCASE("empty") {}
    SUBCASE("partial") {
      REQUIRE(!builder.add(first));
      builder.reset();
    }
    REQUIRE(!builder.add(middle));
    REQUIRE(!builder.add(last));
    REQUIRE(!builder.add(first));
    REQUIRE(!builder.add(middle));
    auto message = builder.add(last);
    REQUIRE(message.has_value());
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }
}
