#ifndef NEXUS_RISK_WEB_SERVLET_HPP
#define NEXUS_RISK_WEB_SERVLET_HPP
#include <unordered_set>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Stomp/StompServer.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <Beam/WebServices/SessionStore.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "WebPortal/WebPortal.hpp"
#include "WebPortal/WebPortalSession.hpp"
#include "WebPortal/PortfolioModel.hpp"

namespace Nexus::WebPortal {

  /** Provides a web interface to the RiskService. */
  class RiskWebServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      /**
       * Constructs a RiskWebServlet.
       * @param sessions The available web sessions.
       * @param serviceClients The clients used to access Spire services.
       */
      RiskWebServlet(Beam::Ref<
        Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        ServiceClientsBox serviceClients);

      ~RiskWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      std::vector<Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>>
        GetWebSocketSlots();

      void Close();

    private:
      using StompServer =
        Beam::Stomp::StompServer<std::unique_ptr<WebSocketChannel>>;
      struct PortfolioFilter {
        std::unordered_set<Beam::ServiceLocator::DirectoryEntry> m_groups;
        std::unordered_set<MarketCode> m_markets;
        std::unordered_set<CurrencyId> m_currencies;

        bool IsFiltered(const PortfolioModel::Entry& entry,
          const Beam::ServiceLocator::DirectoryEntry& group) const;
      };
      struct PortfolioSubscriber {
        Beam::ServiceLocator::DirectoryEntry m_account;
        StompServer m_client;
        std::string m_subscriptionId;
        PortfolioFilter m_filter;

        PortfolioSubscriber(Beam::ServiceLocator::DirectoryEntry account,
          std::unique_ptr<WebSocketChannel> channel);
      };
      ServiceClientsBox m_serviceClients;
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      std::unordered_map<RiskService::RiskPortfolioKey, PortfolioModel::Entry>
        m_portfolioEntries;
      std::unordered_set<PortfolioModel::Entry> m_updatedPortfolioEntries;
      std::vector<std::shared_ptr<PortfolioSubscriber>> m_porfolioSubscribers;
      PortfolioModel m_portfolioModel;
      Beam::Threading::TimerBox m_portfolioTimer;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry> m_traderGroups;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      RiskWebServlet(const RiskWebServlet&) = delete;
      RiskWebServlet& operator =(const RiskWebServlet&) = delete;
      const Beam::ServiceLocator::DirectoryEntry& FindTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& trader);
      void SendPortfolioEntry(const PortfolioModel::Entry& entry,
        const Beam::ServiceLocator::DirectoryEntry& group,
        PortfolioSubscriber& subscriber, bool checkFilter);
      void OnPortfolioUpgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void OnPortfolioRequest(
        const std::shared_ptr<PortfolioSubscriber>& subscriber,
        const Beam::Stomp::StompFrame& frame);
      void OnPortfolioFilterRequest(
        const std::shared_ptr<PortfolioSubscriber>& subscriber,
        const Beam::Stomp::StompFrame& frame);
      void OnPortfolioUpdate(const PortfolioModel::Entry& entry);
      void OnPortfolioTimerExpired(Beam::Threading::Timer::Result result);
  };
}

#endif
