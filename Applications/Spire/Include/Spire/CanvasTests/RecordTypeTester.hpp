#ifndef SPIRE_RECORDTYPETESTER_HPP
#define SPIRE_RECORDTYPETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Spire/CanvasTests/CanvasTests.hpp"

namespace Spire {
namespace Tests {

  /*  \class RecordTypeTester
      \brief Tests the RecordType class.
   */
  class RecordTypeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an empty RecordType.
      void TestEmptyRecord();

    private:
      CPPUNIT_TEST_SUITE(RecordTypeTester);
        CPPUNIT_TEST(TestEmptyRecord);
      CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
