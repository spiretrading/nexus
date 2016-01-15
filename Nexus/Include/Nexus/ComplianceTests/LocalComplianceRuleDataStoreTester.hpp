#ifndef NEXUS_LOCALCOMPLIANCERULEDATASTORETESTER_HPP
#define NEXUS_LOCALCOMPLIANCERULEDATASTORETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class LocalComplianceRuleDataStoreTester
      \brief Tests the LocalComplianceRuleDataStore class.
   */
  class LocalComplianceRuleDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Load entry with ID 123, the result should be not initialized.
      //! Insert an entry with ID 123.
      //! Load entry with ID 123, the result should match the previous insert.
      //! Delete entry with ID 123.
      //! Load entry with ID 123, the result should be not initialized.
      void TestQueriesById();

      //! Load all entries for account with ID 10, the result should be empty.
      //! Insert an entry with ID 123 for account 10.
      //! Load all entries for account with ID 10, the result should contain
      //!   a single entry 123.
      //! Load all entries for account with ID 11, the result should be empty.
      //! Insert an entry with ID 124 for account 11.
      //! Load all entries for account with ID 10, the result should contain
      //!   a single entry 123.
      //! Load all entries for account with ID 11, the result should contain
      //!   a single entry 124.
      //! Insert an entry with ID 123 for account 11.
      //! Load all entries for account with ID 10, the result should be empty.
      //! Load all entries for account with ID 11, the result should contain
      //!   entries 123, 124.
      void TestLoadsByDirectoryEntry();

      //! Insert an entry with ID 123 for account 10.
      //! Insert an entry with ID 124 for account 10.
      //! Load all entries for account with ID 10, the result should contain
      //!   entries 123 and 124.
      //! Delete entry with ID 124.
      //! Load all entries for account with ID 10, the result should contain
      //!   entry 123.
      //! Delete entry with ID 123.
      //! Load all entries for account with ID 10, the result should be empty.
      //! Delete entry with ID 122.
      void TestDeleteByDirectoryEntry();

    private:
      CPPUNIT_TEST_SUITE(LocalComplianceRuleDataStoreTester);
        CPPUNIT_TEST(TestQueriesById);
        CPPUNIT_TEST(TestLoadsByDirectoryEntry);
        CPPUNIT_TEST(TestDeleteByDirectoryEntry);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
