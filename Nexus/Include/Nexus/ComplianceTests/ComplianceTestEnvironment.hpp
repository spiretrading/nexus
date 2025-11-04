#ifndef NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#define NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/ServiceComplianceClient.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

namespace Nexus::Tests {

  /**
   * Wraps most components needed to run an instance of Compliance with
   * helper functions.
   */
  class ComplianceTestEnvironment {
    public:

      /**
       * Constructs a ComplianceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       * @param administration_client The AdministrationClient to use.
       * @param time_client The TimeClient to use.
       */
      ComplianceTestEnvironment(
        Beam::ServiceLocatorClientBox service_locator_client,
        AdministrationClient administration_client,
        Beam::TimeService::TimeClientBox time_client);

      ~ComplianceTestEnvironment();

      /**
       * Returns a new ComplianceClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the ComplianceClient.
       */
      ComplianceClient make_client(
        Beam::ServiceLocatorClientBox service_locator_client);

      void close();

    private:
      using ServerConnection =
        Beam::LocalServerConnection<Beam::SharedBuffer>;
      using ClientChannel =
        Beam::LocalClientChannel<Beam::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<
            MetaComplianceServlet<Beam::ServiceLocatorClientBox,
              AdministrationClient, LocalComplianceRuleDataStore*,
              Beam::TimeService::TimeClientBox>,
            Beam::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::BinarySender<Beam::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocatorClientBox,
          Beam::MessageProtocol<std::unique_ptr<ClientChannel>,
            Beam::BinarySender<Beam::SharedBuffer>,
            Beam::Codecs::NullEncoder>,
          Beam::TriggerTimer>;
      ServerConnection m_server_connection;
      LocalComplianceRuleDataStore m_data_store;
      ServiceProtocolServletContainer m_container;

      ComplianceTestEnvironment(const ComplianceTestEnvironment&) = delete;
      ComplianceTestEnvironment& operator =(
        const ComplianceTestEnvironment&) = delete;
  };

  inline ComplianceTestEnvironment::ComplianceTestEnvironment(
    Beam::ServiceLocatorClientBox service_locator_client,
    AdministrationClient administration_client,
    Beam::TimeService::TimeClientBox time_client)
    : m_container(Beam::Initialize(service_locator_client, Beam::Initialize(
        service_locator_client, std::move(administration_client), &m_data_store,
        std::move(time_client))), &m_server_connection,
        boost::factory<std::shared_ptr<Beam::TriggerTimer>>()) {}

  inline ComplianceTestEnvironment::~ComplianceTestEnvironment() {
    close();
  }

  inline ComplianceClient ComplianceTestEnvironment::make_client(
      Beam::ServiceLocatorClientBox service_locator_client) {
    return ComplianceClient(
      std::in_place_type<ServiceComplianceClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(service_locator_client),
        std::bind_front(boost::factory<std::unique_ptr<
          ServiceProtocolClientBuilder::Channel>>(), "test_compliance_client",
          std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void ComplianceTestEnvironment::close() {
    m_container.Close();
  }
}

#endif
