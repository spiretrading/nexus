#include <ostream>
#include <doctest/doctest.h>
#include "Nexus/Stamp/StampMessage.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("StampMessage") {
  TEST_CASE("parse") {
    auto source = "\x01\x1e" "56=20260919070000"
      "\x1c\x1e" "6=OpeningAuction\x1e" "55=ABX\x1e" "191=25.50"sv;
    SUBCASE("without_trailer") {}
    SUBCASE("with_trailer") {
      source = "\x01\x1e" "56=20260919070000"
        "\x1c\x1e" "6=OpeningAuction\x1e" "55=ABX\x1e" "191=25.50"
        "\x1d"sv;
    }
    auto message = StampMessage::parse(source);
    auto timestamp = message.m_control_header.find(56);
    REQUIRE(timestamp.has_value());
    REQUIRE(timestamp->m_value == "20260919070000");
    auto symbol = message.m_business_content.find(55);
    REQUIRE(symbol.has_value());
    REQUIRE(symbol->m_value == "ABX");
    REQUIRE(symbol->m_value.data() == source.data() + source.find("ABX"));
    auto price = message.m_business_content.find(191);
    REQUIRE(price.has_value());
    REQUIRE(price->m_value == "25.50");
    REQUIRE(!message.m_control_header.find(55));
    REQUIRE(!message.m_business_content.find(56));
    auto i = message.m_business_content.begin();
    REQUIRE(i->m_identifier == 6);
    REQUIRE((i++)->m_value == "OpeningAuction");
    REQUIRE((*i).m_identifier == 55);
    REQUIRE((++i)->m_identifier == 191);
    ++i;
    REQUIRE(i == message.m_business_content.end());
  }

  TEST_CASE("field_lookup") {
    auto source = "\x01\x1e" "55=header\x1e" "56=20260919070000"
      "\x1c\x1e" "55.1=XYZ\x1e" "41.0=25.50\x1e" "55=ABX"
      "\x1e" "9999=unknown\x1e" "64.1="sv;
    SUBCASE("unordered") {}
    SUBCASE("reordered") {
      source = "\x01\x1e" "56=20260919070000\x1e" "55=header"
        "\x1c\x1e" "64.1=\x1e" "9999=unknown\x1e" "55=ABX"
        "\x1e" "41.0=25.50\x1e" "55.1=XYZ"sv;
    }
    auto message = StampMessage::parse(source);
    auto symbol = message.m_business_content.find(55);
    REQUIRE(symbol.has_value());
    REQUIRE(symbol->m_index == 0);
    REQUIRE(symbol->m_value == "ABX");
    auto first = message.m_business_content.find(55, 0);
    REQUIRE(first.has_value());
    REQUIRE(first->m_value == symbol->m_value);
    auto second = message.m_business_content.find(55, 1);
    REQUIRE(second.has_value());
    REQUIRE(second->m_value == "XYZ");
    REQUIRE(!message.m_business_content.find(55, 2));
    REQUIRE(!message.m_business_content.find(41, 1));
    auto header = message.m_control_header.find(55);
    REQUIRE(header.has_value());
    REQUIRE(header->m_value == "header");
    auto empty = message.m_business_content.find(64, 1);
    REQUIRE(empty.has_value());
    REQUIRE(empty->m_value.empty());
    REQUIRE(!message.m_business_content.find(64));
    auto unknown = message.m_business_content.find(9999);
    REQUIRE(unknown.has_value());
    REQUIRE(unknown->m_value == "unknown");
    auto count = 0;
    for(auto& field : message.m_business_content) {
      REQUIRE(field.m_identifier != 0);
      ++count;
    }
    REQUIRE(count == 5);
  }

  TEST_CASE("malformed_message") {
    for(auto source : {""sv, "\x01"sv, "\x01\x1c"sv,
        "\x1e" "56=t\x1c\x1e" "55=ABX"sv,
        "\x01\x1e" "56=t"sv, "\x01\x1c\x1e" "55=ABX"sv,
        "\x01\x1e" "56=t\x1c"sv,
        "\x01\x1e" "56=t\x1c\x1d"sv,
        "\x01" "56=t\x1c\x1e" "55=ABX"sv,
        "\x01\x1e" "56=t\x1c" "55=ABX"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1e"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1e\x1e" "41=25"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1e" "41.X=25"sv,
        "\x01\x1e" "56=t\x1e" "0=x\x1c\x1e" "55=ABX"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x01"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1c\x1e" "41=25"sv,
        "\x01\x1e" "56=t\x1d\x1c\x1e" "55=ABX"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1d" "junk"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\x1d\x1d"sv,
        "\x02\x01\x1e" "56=t\x1c\x1e" "55=ABX\x03"sv,
        "\x01\x1e" "56=t\x1c\x1e" "55=ABX\0"sv}) {
      CAPTURE(source);
      REQUIRE_THROWS_AS(StampMessage::parse(source), StampParserException);
    }
  }
}
