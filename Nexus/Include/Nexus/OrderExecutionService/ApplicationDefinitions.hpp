#ifndef NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#include <optional>
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/SizeDeclarativeReader.hpp>
#include <Beam/IO/SizeDeclarativeWriter.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Details {
  using OrderExecutionClientSessionBuilder =
    Beam::Services::AuthenticatedServiceProtocolClientBuilder<
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
    Beam::Services::MessageProtocol<
    std::unique_ptr<Beam::Network::TcpSocketChannel>,
    Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
    Beam::Codecs::NullEncoder>, Beam::Threading::LiveTimer>;
}

  /*! \class ApplicationOrderExecutionClient
      \brief Encapsulates a standard OrderExecutionClient used in an
             application.
   */
  class ApplicationOrderExecutionClient : private boost::noncopyable {
    public:

      //! Defines the standard OrderExecutionClient used for applications.
      using Client = OrderExecutionClient<
        Details::OrderExecutionClientSessionBuilder>;

      //! Constructs an ApplicationOrderExecutionClient.
      ApplicationOrderExecutionClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
      */
      void BuildSession(Beam::Ref<Beam::ServiceLocator::
        ApplicationServiceLocatorClient::Client> serviceLocatorClient);

      //! Returns a reference to the Client.
      Client& operator *();

      //! Returns a reference to the Client.
      const Client& operator *() const;

      //! Returns a pointer to the Client.
      Client* operator ->();

      //! Returns a pointer to the Client.
      const Client* operator ->() const;

      //! Returns a pointer to the Client.
      Client* Get();

      //! Returns a pointer to the Client.
      const Client* Get() const;

    private:
      std::optional<Client> m_client;
  };

  inline void ApplicationOrderExecutionClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    if(m_client.has_value()) {
      m_client->Close();
      m_client = std::nullopt;
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, SERVICE_NAME);
    auto delay = false;
    auto sessionBuilder = Details::OrderExecutionClientSessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = Beam::Threading::LiveTimer(
            boost::posix_time::seconds(3));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(sessionBuilder);
  }

  inline ApplicationOrderExecutionClient::Client&
      ApplicationOrderExecutionClient::operator *() {
    return *m_client;
  }

  inline const ApplicationOrderExecutionClient::Client&
      ApplicationOrderExecutionClient::operator *() const {
    return *m_client;
  }

  inline ApplicationOrderExecutionClient::Client*
      ApplicationOrderExecutionClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationOrderExecutionClient::Client*
      ApplicationOrderExecutionClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationOrderExecutionClient::Client*
      ApplicationOrderExecutionClient::Get() {
    return &*m_client;
  }

  inline const ApplicationOrderExecutionClient::Client*
      ApplicationOrderExecutionClient::Get() const {
    return &*m_client;
  }
}
}

#endif
