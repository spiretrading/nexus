#ifndef NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#define NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceClientBox.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus::Compliance::Tests {

  /**
   * Wraps most components needed to run an instance of Compliance with
   * helper functions.
   */
  class ComplianceTestEnvironment {
    public:

      /**
       * Constructs a ComplianceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param administrationClient The AdministrationClient to use.
       * @param timeClient The TimeClient to use.
       */
      ComplianceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        AdministrationService::AdministrationClientBox administrationClient,
        Beam::TimeService::TimeClientBox timeClient);

      ~ComplianceTestEnvironment();

      /**
       * Returns a new ComplianceClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the ComplianceClient.
       */
      ComplianceClientBox MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaComplianceServlet<Beam::ServiceLocator::ServiceLocatorClientBox,
              AdministrationService::AdministrationClientBox,
              LocalComplianceRuleDataStore*, Beam::TimeService::TimeClientBox>,
            Beam::ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocator::ServiceLocatorClientBox,
          Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
            Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
            Beam::Codecs::NullEncoder>,
          Beam::Threading::TriggerTimer>;
      ServerConnection m_serverConnection;
      LocalComplianceRuleDataStore m_dataStore;
      ServiceProtocolServletContainer m_container;

      ComplianceTestEnvironment(const ComplianceTestEnvironment&) = delete;
      ComplianceTestEnvironment& operator =(
        const ComplianceTestEnvironment&) = delete;
  };

  inline ComplianceTestEnvironment::ComplianceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
    AdministrationService::AdministrationClientBox administrationClient,
    Beam::TimeService::TimeClientBox timeClient)
    : m_container(Beam::Initialize(serviceLocatorClient, Beam::Initialize(
        serviceLocatorClient, std::move(administrationClient), &m_dataStore,
        std::move(timeClient))), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline ComplianceTestEnvironment::~ComplianceTestEnvironment() {
    Close();
  }

  inline ComplianceClientBox ComplianceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return ComplianceClientBox(
      std::in_place_type<ComplianceClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
        std::bind(boost::factory<std::unique_ptr<
          ServiceProtocolClientBuilder::Channel>>(), "test_compliance_client",
          std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void ComplianceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
