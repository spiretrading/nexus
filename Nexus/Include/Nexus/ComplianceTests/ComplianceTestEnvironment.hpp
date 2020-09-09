#ifndef NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#define NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
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
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient,
        std::shared_ptr<Beam::TimeService::VirtualTimeClient> timeClient);

      ~ComplianceTestEnvironment();

      /**
       * Builds a new ComplianceClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the ComplianceClient.
       */
      std::unique_ptr<VirtualComplianceClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      /** Closes the servlet. */
      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using UidClient = Beam::UidService::VirtualUidClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaComplianceServlet<std::shared_ptr<ServiceLocatorClient>,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>,
        LocalComplianceRuleDataStore*,
        std::shared_ptr<Beam::TimeService::VirtualTimeClient>>,
        std::shared_ptr<ServiceLocatorClient>>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      ServerConnection m_serverConnection;
      LocalComplianceRuleDataStore m_dataStore;
      ServiceProtocolServletContainer m_container;

      ComplianceTestEnvironment(const ComplianceTestEnvironment&) = delete;
      ComplianceTestEnvironment& operator =(
        const ComplianceTestEnvironment&) = delete;
  };

  inline ComplianceTestEnvironment::ComplianceTestEnvironment(
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient,
    std::shared_ptr<AdministrationService::VirtualAdministrationClient>
    administrationClient,
    std::shared_ptr<Beam::TimeService::VirtualTimeClient> timeClient)
    : m_container(Beam::Initialize(serviceLocatorClient, Beam::Initialize(
        serviceLocatorClient, std::move(administrationClient), &m_dataStore,
        std::move(timeClient))), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline ComplianceTestEnvironment::~ComplianceTestEnvironment() {
    Close();
  }

  inline std::unique_ptr<VirtualComplianceClient>
      ComplianceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_compliance_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    return MakeVirtualComplianceClient(
      std::make_unique<ComplianceClient<ServiceProtocolClientBuilder>>(
      builder));
  }

  inline void ComplianceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
