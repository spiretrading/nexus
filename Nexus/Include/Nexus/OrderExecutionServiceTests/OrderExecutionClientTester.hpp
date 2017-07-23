#ifndef NEXUS_ORDEREXECUTIONCLIENTTESTER_HPP
#define NEXUS_ORDEREXECUTIONCLIENTTESTER_HPP
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class OrderExecutionClientTester
      \brief Tests the OrderExecutionClient class.
   */
  class OrderExecutionClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of OrderExecutionClient.
      using TestOrderExecutionClient = OrderExecutionClient<
        Beam::Services::Tests::TestServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests submitting an Order.
      void TestSubmitOrder();

    private:
      boost::optional<Beam::Services::Tests::TestServiceProtocolServer>
        m_server;
      boost::optional<TestOrderExecutionClient> m_client;

      CPPUNIT_TEST_SUITE(OrderExecutionClientTester);
        CPPUNIT_TEST(TestSubmitOrder);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
