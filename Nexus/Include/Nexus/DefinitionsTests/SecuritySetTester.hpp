#ifndef NEXUS_SECURITYSETTESTER_HPP
#define NEXUS_SECURITYSETTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class SecuritySetTester
      \brief Tests the SecuritySet class.
   */
  class SecuritySetTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a concrete Security.
      void TestParsingConcreteSecurity();

      //! Tests parsing a Security with a wild-card market.
      void TestParsingWildcardMarket();

      //! Tests parsing a Security with a wild-card symbol.
      void TestParsingWildcardSymbol();

      //! Tests parsing a Security with a wild-card symbol and market.
      void TestParsingWildcardSymbolAndMarket();

      //! Tests parsing a Security with a wild-card symbol and market and
      //! concrete country.
      void TestParsingWildcardSymbolAndMarketConcreteCountry();

      //! Tests parsing a wild card on a specific country.
      void TestParsingWildcardConcreteCountry();

      //! Tests a single concrete Security.
      void TestConcreteSecurity();

    private:
      CPPUNIT_TEST_SUITE(SecuritySetTester);
        CPPUNIT_TEST(TestParsingConcreteSecurity);
        CPPUNIT_TEST(TestParsingWildcardMarket);
        CPPUNIT_TEST(TestParsingWildcardSymbol);
        CPPUNIT_TEST(TestParsingWildcardSymbolAndMarket);
        CPPUNIT_TEST(TestParsingWildcardSymbolAndMarketConcreteCountry);
        CPPUNIT_TEST(TestParsingWildcardConcreteCountry);
        CPPUNIT_TEST(TestConcreteSecurity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
