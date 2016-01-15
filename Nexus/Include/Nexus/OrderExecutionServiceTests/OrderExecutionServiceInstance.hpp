#ifndef NEXUS_ORDEREXECUTIONSERVICETESTINSTANCE_HPP
#define NEXUS_ORDEREXECUTIONSERVICETESTINSTANCE_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class OrderExecutionServiceTestInstance
      \brief Wraps most components needed to run an instance of the
             OrderExecutionService with helper functions.
   */
  class OrderExecutionServiceTestInstance : private boost::noncopyable {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient =
        Beam::ServiceLocator::Tests::ServiceLocatorTestInstance::
        ServiceLocatorClient;

      //! The type of UidClient used.
      using UidClient =
        Beam::UidService::Tests::UidServiceTestInstance::UidClient;

      //! The type of AdministrationClient used.
      using AdministrationClient =
        AdministrationService::Tests::AdministrationServiceTestInstance::
        AdministrationClient;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaOrderExecutionServlet<Beam::TimeService::IncrementalTimeClient,
        std::shared_ptr<ServiceLocatorClient>, std::unique_ptr<UidClient>,
        std::unique_ptr<AdministrationClient>, MockOrderExecutionDriver*,
        LocalOrderExecutionDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build OrderExecutionClient sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of OrderExecutionClient used.
      using OrderExecutionClient = OrderExecutionService::OrderExecutionClient<
        ServiceProtocolClientBuilder>;

      //! Constructs an OrderExecutionServiceTestInstance.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
        \param uidClient The UidClient to use.
      */
      OrderExecutionServiceTestInstance(
        const std::shared_ptr<ServiceLocatorClient>& serviceLocatorClient,
        std::unique_ptr<UidClient> uidClient,
        std::unique_ptr<AdministrationClient> administrationClient);

      ~OrderExecutionServiceTestInstance();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Returns the driver used to manage submitted Orders.
      MockOrderExecutionDriver& GetDriver();

      //! Builds a new OrderExecutionClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the OrderExecutionClient.
      */
      std::unique_ptr<OrderExecutionClient> BuildClient(
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient);

    private:
      LocalOrderExecutionDataStore m_dataStore;
      MockOrderExecutionDriver m_driver;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
  };

  inline OrderExecutionServiceTestInstance::OrderExecutionServiceTestInstance(
      const std::shared_ptr<ServiceLocatorClient>& serviceLocatorClient,
      std::unique_ptr<UidClient> uidClient,
      std::unique_ptr<AdministrationClient> administrationClient)
      : m_container(Beam::Initialize(serviceLocatorClient,
          Beam::Initialize(boost::posix_time::pos_infin, Beam::Initialize(),
          serviceLocatorClient, std::move(uidClient),
          std::move(administrationClient), &m_driver, &m_dataStore)),
          &m_serverConnection,
          boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline OrderExecutionServiceTestInstance::
      ~OrderExecutionServiceTestInstance() {
    Close();
  }

  inline void OrderExecutionServiceTestInstance::Open() {
    m_container.Open();
  }

  inline void OrderExecutionServiceTestInstance::Close() {
    m_container.Close();
  }

  inline MockOrderExecutionDriver& OrderExecutionServiceTestInstance::
      GetDriver() {
    return m_driver;
  }

  inline std::unique_ptr<OrderExecutionServiceTestInstance::
      OrderExecutionClient> OrderExecutionServiceTestInstance::BuildClient(
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient) {
    ServiceProtocolClientBuilder builder(Beam::Ref(serviceLocatorClient),
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_order_execution_client", Beam::Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<OrderExecutionClient>(builder);
    return client;
  }
}
}
}

#endif
