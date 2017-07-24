#ifndef NEXUS_ORDEREXECUTIONSERVLETTESTER_HPP
#define NEXUS_ORDEREXECUTIONSERVLETTESTER_HPP
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class OrderExecutionServletTester
      \brief Tests the OrderExecutionServlet class.
   */
  class OrderExecutionServletTester: public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolServletContainer.
      using ServletContainer =
        Beam::Services::Tests::TestAuthenticatedServiceProtocolServletContainer<
        MetaOrderExecutionServlet<Beam::TimeService::IncrementalTimeClient,
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>,
        std::unique_ptr<Beam::UidService::VirtualUidClient>,
        std::unique_ptr<AdministrationService::VirtualAdministrationClient>,
        std::shared_ptr<MockOrderExecutionDriver>,
        std::shared_ptr<LocalOrderExecutionDataStore>>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests a NewOrderSingleMessage.
      void TestNewOrderSingle();

    private:
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      boost::optional<Beam::UidService::Tests::UidServiceTestEnvironment>
        m_uidServiceEnvironment;
      boost::optional<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationServiceEnvironment;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_clientServiceLocatorClient;
      std::shared_ptr<MockOrderExecutionDriver> m_driver;
      std::shared_ptr<LocalOrderExecutionDataStore> m_dataStore;
      boost::optional<ServletContainer> m_container;
      boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

      CPPUNIT_TEST_SUITE(OrderExecutionServletTester);
        CPPUNIT_TEST(TestNewOrderSingle);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
