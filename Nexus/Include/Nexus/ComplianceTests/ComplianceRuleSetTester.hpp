#ifndef NEXUS_COMPLIANCERULESETTESTER_HPP
#define NEXUS_COMPLIANCERULESETTESTER_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class ComplianceRuleSetTester
      \brief Tests the ComplianceRuleSet class.
   */
  class ComplianceRuleSetTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient = Beam::ServiceLocator::Tests::
        ServiceLocatorTestInstance::ServiceLocatorClient;

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer =
        Beam::Services::ServiceProtocolServer<ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::ServiceProtocolClientBuilder<
        Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of ComplianceClient.
      using TestComplianceClient =
        ComplianceClient<ServiceProtocolClientBuilder>;

      //! The type of ComplianceRuleSet to test.
      using TestComplianceRuleSet = ComplianceRuleSet<
        TestComplianceClient*, std::unique_ptr<ServiceLocatorClient>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests submitting an Order.
      void TestSubmit();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServiceProtocolServer> m_server;
      Beam::DelayPtr<TestComplianceClient> m_complianceClient;
      Beam::DelayPtr<TestComplianceRuleSet> m_complianceRuleSet;

      CPPUNIT_TEST_SUITE(ComplianceRuleSetTester);
        CPPUNIT_TEST(TestSubmit);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
