#include "Nexus/ParsersTests/SecurityParserTester.hpp"
#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;
using namespace Nexus::Tests;

void SecurityParserTester::TestWellFormedSecurity() {
  SecurityParser parser(GetDefaultMarketDatabase());
  auto stream = ParserStreamFromString("ABX.TSX");
  Security security;
  CPPUNIT_ASSERT(parser.Read(stream, security));
  CPPUNIT_ASSERT(security.GetSymbol() == "ABX");
  CPPUNIT_ASSERT(security.GetMarket() == DefaultMarkets::TSX());
  CPPUNIT_ASSERT(security.GetCountry() == DefaultCountries::CA());
  stream = ParserStreamFromString("ABX.XST");
  CPPUNIT_ASSERT(!parser.Read(stream, security));
}

void SecurityParserTester::TestUpperCaseSecurity() {
  SecurityParser parser(GetDefaultMarketDatabase());
  auto stream = ParserStreamFromString("aBx.TsX");
  Security security;
  CPPUNIT_ASSERT(parser.Read(stream, security));
  CPPUNIT_ASSERT(security.GetSymbol() == "ABX");
  CPPUNIT_ASSERT(security.GetMarket() == DefaultMarkets::TSX());
  CPPUNIT_ASSERT(security.GetCountry() == DefaultCountries::CA());
}
