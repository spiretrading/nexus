#ifndef NEXUS_RISKWEBSERVLET_HPP
#define NEXUS_RISKWEBSERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>
#include <boost/noncopyable.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortal.hpp"
#include "ClientWebPortal/ClientWebPortal/ClientWebPortalSession.hpp"
#include "ClientWebPortal/ClientWebPortal/PortfolioModel.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class RiskWebServlet
      \brief Provides a web interface to the RiskService.
   */
  class RiskWebServlet : private boost::noncopyable {
    public:

      //! The type of WebSocketChannel used.
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      //! Constructs a RiskWebServlet.
      /*!
        \param sessions The available web sessions.
        \param serviceClients The clients used to access Spire services.
      */
      RiskWebServlet(Beam::RefType<
        Beam::WebServices::SessionStore<ClientWebPortalSession>> sessions,
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~RiskWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      std::vector<Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>>
        GetWebSocketSlots();

      void Open();

      void Close();

    private:
      using StompServer =
        Beam::Stomp::StompServer<std::unique_ptr<WebSocketChannel>>;
      struct PortfolioSubscriber {
        std::shared_ptr<StompServer> m_client;
        std::string m_subscriptionId;
      };
      ApplicationServiceClients* m_serviceClients;
      Beam::WebServices::SessionStore<ClientWebPortalSession>* m_sessions;
      std::unique_ptr<ApplicationServiceClients::Timer> m_portfolioTimer;
      std::unordered_map<RiskService::RiskPortfolioKey, PortfolioModel::Entry>
        m_portfolioEntries;
      std::unordered_set<PortfolioModel::Entry> m_updatedPortfolioEntries;
      std::vector<std::shared_ptr<PortfolioSubscriber>> m_porfolioSubscribers;
      PortfolioModel m_portfolioModel;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnPortfolioUpgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void OnPortfolioUpdate(const PortfolioModel::Entry& entry);
      void OnPortfolioTimerExpired(Beam::Threading::Timer::Result result);
  };
}
}

#endif
