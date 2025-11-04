#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/OrderStatusParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OrderStatusParser") {
  TEST_CASE("all_order_statuses") {
    auto parser = order_status_parser();
    auto cases = std::array{
      std::tuple("PENDING_NEW", OrderStatus::PENDING_NEW),
      std::tuple("REJECTED", OrderStatus::REJECTED),
      std::tuple("NEW", OrderStatus::NEW),
      std::tuple("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED),
      std::tuple("EXPIRED", OrderStatus::EXPIRED),
      std::tuple("CANCELED", OrderStatus::CANCELED),
      std::tuple("SUSPENDED", OrderStatus::SUSPENDED),
      std::tuple("STOPPED", OrderStatus::STOPPED),
      std::tuple("FILLED", OrderStatus::FILLED),
      std::tuple("DONE_FOR_DAY", OrderStatus::DONE_FOR_DAY),
      std::tuple("PENDING_CANCEL", OrderStatus::PENDING_CANCEL),
      std::tuple("CANCEL_REJECT", OrderStatus::CANCEL_REJECT)};
    for(auto& entry : cases) {
      auto stream = to_parser_stream(std::get<0>(entry));
      auto status = OrderStatus();
      REQUIRE(parser.read(stream, status));
      REQUIRE(status == std::get<1>(entry));
    }
  }

  TEST_CASE("invalid_order_status") {
    auto parser = order_status_parser();
    auto stream = to_parser_stream("INVALID_STATUS");
    auto status = OrderStatus();
    REQUIRE_FALSE(parser.read(stream, status));
  }

  TEST_CASE("lowercase_order_status") {
    auto parser = order_status_parser();
    auto stream = to_parser_stream("new");
    auto status = OrderStatus();
    REQUIRE_FALSE(parser.read(stream, status));
  }

  TEST_CASE("partial_order_status") {
    auto parser = order_status_parser();
    auto stream = to_parser_stream("PENDING");
    auto status = OrderStatus();
    REQUIRE_FALSE(parser.read(stream, status));
  }

  TEST_CASE("multiple_order_statuses") {
    auto parser = order_status_parser();
    auto stream = to_parser_stream("NEWFILLED");
    auto status = OrderStatus();
    REQUIRE(parser.read(stream, status));
    REQUIRE(status == OrderStatus::NEW);
    REQUIRE(parser.read(stream, status));
    REQUIRE(status == OrderStatus::FILLED);
  }
}
