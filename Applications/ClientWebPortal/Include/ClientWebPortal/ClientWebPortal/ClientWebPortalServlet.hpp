#ifndef NEXUS_CLIENTWEBPORTALSERVLET_HPP
#define NEXUS_CLIENTWEBPORTALSERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/FileStore.hpp>
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

  /*! \class ClientWebPortalServlet
      \brief Implements a web servlet for Spire client services.
   */
  class ClientWebPortalServlet : private boost::noncopyable {
    public:

      //! The type of WebSocketChannel used.
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      //! Constructs a ClientWebPortalServlet.
      /*!
        \param serviceClients The clients used to access Spire services.
      */
      ClientWebPortalServlet(
        Beam::RefType<ApplicationServiceClients> serviceClients);

      ~ClientWebPortalServlet();

      //! Returns the HTTP request slots.
      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      //! Returns the WebSocket upgrade slots.
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
      Beam::WebServices::FileStore m_fileStore;
      Beam::WebServices::SessionStore<ClientWebPortalSession> m_sessions;
      ApplicationServiceClients* m_serviceClients;
      std::unique_ptr<ApplicationServiceClients::Timer> m_portfolioTimer;
      std::unordered_map<RiskService::RiskPortfolioKey, PortfolioModel::Entry>
        m_portfolioEntries;
      std::unordered_set<PortfolioModel::Entry> m_updatedPortfolioEntries;
      std::vector<std::shared_ptr<PortfolioSubscriber>> m_porfolioSubscribers;
      PortfolioModel m_portfolioModel;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      Beam::WebServices::HttpResponse OnIndex(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnServeFile(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrentAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStorePassword(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnSearchDirectoryEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogin(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLogout(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnCreateAccount(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnCreateGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadTradingGroup(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadManagedTradingGroups(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountRoles(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountIdentity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreAccountIdentity(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadEntitlementsDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadAccountEntitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreAccountEntitlements(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadRiskParameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnStoreRiskParameters(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadDirectoryEntryComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnAddComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnUpdateComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnDeleteComplianceRuleEntry(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadComplianceRuleSchemas(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCountryDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadCurrencyDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadMarketDatabase(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadSecurityInfoFromPrefix(
        const Beam::WebServices::HttpRequest& request);
      Beam::WebServices::HttpResponse OnLoadProfitAndLossReport(
        const Beam::WebServices::HttpRequest& request);
      void OnPortfolioUpgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void OnPortfolioUpdate(const PortfolioModel::Entry& entry);
      void OnPortfolioTimerExpired(Beam::Threading::Timer::Result result);
  };
}
}

#endif
