#ifndef NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#define NEXUS_COMPLIANCE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
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
        Beam::ServiceLocatorClient service_locator_client,
        AdministrationClient administration_client,
        Beam::TimeClient time_client);

      ~ComplianceTestEnvironment();

      /**
       * Returns a new ComplianceClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the ComplianceClient.
       */
      ComplianceClient make_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client);

      void close();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<
            MetaComplianceServlet<Beam::ServiceLocatorClient,
              AdministrationClient, LocalComplianceRuleDataStore*,
              Beam::TimeClient>, Beam::ServiceLocatorClient>,
          Beam::LocalServerConnection*, Beam::BinarySender<Beam::SharedBuffer>,
          Beam::NullEncoder, std::shared_ptr<Beam::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocatorClient,
          Beam::MessageProtocol<std::unique_ptr<Beam::LocalClientChannel>,
            Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder>,
          Beam::TriggerTimer>;
      Beam::LocalServerConnection m_server_connection;
      LocalComplianceRuleDataStore m_data_store;
      ServiceProtocolServletContainer m_container;

      ComplianceTestEnvironment(const ComplianceTestEnvironment&) = delete;
      ComplianceTestEnvironment& operator =(
        const ComplianceTestEnvironment&) = delete;
  };

  inline ComplianceTestEnvironment::ComplianceTestEnvironment(
    Beam::ServiceLocatorClient service_locator_client,
    AdministrationClient administration_client,
    Beam::TimeClient time_client)
    : m_container(Beam::init(service_locator_client, Beam::init(
        service_locator_client, std::move(administration_client), &m_data_store,
        std::move(time_client))), &m_server_connection,
        boost::factory<std::shared_ptr<Beam::TriggerTimer>>()) {}

  inline ComplianceTestEnvironment::~ComplianceTestEnvironment() {
    close();
  }

  inline ComplianceClient ComplianceTestEnvironment::make_client(
      Beam::Ref<Beam::ServiceLocatorClient> service_locator_client) {
    return ComplianceClient(
      std::in_place_type<ServiceComplianceClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(Beam::Ref(service_locator_client),
        std::bind_front(boost::factory<std::unique_ptr<
          ServiceProtocolClientBuilder::Channel>>(), "test_compliance_client",
          std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void ComplianceTestEnvironment::close() {
    m_container.close();
  }
}

#endif
