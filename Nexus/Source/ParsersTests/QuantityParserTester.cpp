#include <limits>
#include <vector>
#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/QuantityParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("QuantityParser") {
  TEST_CASE("valid_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("123.456");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("123.456"));
  }

  TEST_CASE("integer_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("100");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("100"));
  }

  TEST_CASE("negative_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("-42.5");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("-42.5"));
  }

  TEST_CASE("zero_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("0.0");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == Quantity(0));
  }

  TEST_CASE("invalid_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("abc");
    auto quantity = Quantity();
    REQUIRE_FALSE(parser.read(stream, quantity));
  }

  TEST_CASE("multiple_quantities") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("1.5-2.5");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("1.5"));
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("-2.5"));
  }

  TEST_CASE("exact_decimals") {
    for(auto& [text, expected] :
        std::vector<std::pair<std::string, double>>({
          {"1.1", 1100000}, {"1.000001", 1000001},
          {"-1.000001", -1000001}, {"0.000001", 1},
          {"9007199254.740991", 9007199254740991.0},
          {"1.000001e2", 100000100}, {"1000001E-6", 1000001},
          {"0.01000001e2", 1000001}, {"1.0000010", 1000001}})) {
      CAPTURE(text);
      auto stream = to_parser_stream(text + ",");
      auto quantity = Quantity();
      REQUIRE(quantity_parser().read(stream, quantity));
      REQUIRE(quantity.get_representation() == expected);
      REQUIRE(stream.read());
      REQUIRE(stream.peek() == ',');
      stream = to_parser_stream(text + ",");
      REQUIRE(quantity_parser().read(stream));
      REQUIRE(stream.read());
      REQUIRE(stream.peek() == ',');
    }
  }


  TEST_CASE("fractional_representation") {
    for(auto& [text, expected] :
        std::vector<std::pair<std::string, double>>({
          {"0.0000005", 0.5}, {"-0.0000005", -0.5},
          {"1.0000005", 1000000.5}, {"5e-7", 0.5}})) {
      CAPTURE(text);
      auto stream = to_parser_stream(text);
      auto quantity = Quantity();
      REQUIRE(quantity_parser().read(stream, quantity));
      REQUIRE(quantity.get_representation() == expected);
    }
  }

  TEST_CASE("representation_limits") {
    for(auto& [text, expected] :
        std::vector<std::pair<std::string, double>>({
          {"1.7976931348623157e302", std::numeric_limits<double>::max()},
          {"4.9406564584124654e-330",
            std::numeric_limits<double>::denorm_min()},
          {"0e9999", 0}, {"0e-9999", 0}})) {
      CAPTURE(text);
      auto stream = to_parser_stream(text);
      auto quantity = Quantity();
      REQUIRE(quantity_parser().read(stream, quantity));
      REQUIRE(quantity.get_representation() == expected);
    }
  }

  TEST_CASE("invalid_decimals") {
    for(auto& text : {"", "-", "1.", "1e", "1e+", "1e-", "1.e2",
        "1e+x", "1E--2", "1e303", "1e9999", "1e-9999"}) {
      CAPTURE(text);
      auto stream = to_parser_stream(text);
      auto quantity = Quantity(7);
      REQUIRE(!quantity_parser().read(stream, quantity));
      REQUIRE(quantity == Quantity(7));
      if(*text != '\0') {
        REQUIRE(stream.read());
        REQUIRE(stream.peek() == *text);
      }
      stream = to_parser_stream(text);
      REQUIRE(!quantity_parser().read(stream));
      if(*text != '\0') {
        REQUIRE(stream.read());
        REQUIRE(stream.peek() == *text);
      }
    }
  }

}
