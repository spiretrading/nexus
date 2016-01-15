#ifndef NEXUS_SQLTRANSLATORTESTER_HPP
#define NEXUS_SQLTRANSLATORTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus//QueriesTests/QueriesTests.hpp"

namespace Nexus {
namespace Queries {
namespace Tests {

  /*  \class SqlTranslatorTester
      \brief Tests the SqlTranslator class.
   */
  class SqlTranslatorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests querying OrderFields.
      void TestQueryOrderFields();

    private:
      CPPUNIT_TEST_SUITE(SqlTranslatorTester);
        CPPUNIT_TEST(TestQueryOrderFields);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
