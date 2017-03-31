#ifndef NEXUS_COMPLIANCESERVLETTESTER_HPP
#define NEXUS_COMPLIANCESERVLETTESTER_HPP
#include <Beam/Codecs/NullDecoder.hpp>
#include <Beam/Codecs/NullEncoder.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class ComplianceServletTester
      \brief Tests the ComplianceServlet class.
   */
  class ComplianceServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of AdministrationClient.
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      //! The type of ComplianceRuleSet to test.
      using TestComplianceRuleSet = ComplianceRuleSet<
        LocalComplianceRuleDataStore*, std::unique_ptr<ServiceLocatorClient>>;

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaComplianceServlet<std::unique_ptr<ServiceLocatorClient>,
        std::unique_ptr<AdministrationClient>, TestComplianceRuleSet*,
        Beam::TimeService::IncrementalTimeClient>,
        std::unique_ptr<ServiceLocatorClient>>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      virtual void setUp();

      virtual void tearDown();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      Beam::DelayPtr<
        AdministrationService::Tests::AdministrationServiceTestEnvironment>
        m_administrationServiceEnvironment;
      Beam::DelayPtr<LocalComplianceRuleDataStore> m_dataStore;
      Beam::DelayPtr<TestComplianceRuleSet> m_complianceRuleSet;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServletContainer> m_container;

      CPPUNIT_TEST_SUITE(ComplianceServletTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
