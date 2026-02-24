#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/AssetParser.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("AssetParser") {
  TEST_CASE("currency_asset") {
    auto parser = asset_parser();
    auto stream = to_parser_stream("USD");
    auto asset = Asset();
    REQUIRE(parser.read(stream, asset));
    REQUIRE(asset == USD);
  }
}
