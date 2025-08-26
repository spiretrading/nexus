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
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "WebPortal/PortfolioModel.hpp"
#include "WebPortal/WebPortalSession.hpp"

namespace Nexus {

  /** Provides a web interface to the RiskService. */
  class RiskWebServlet {
    public:

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Beam::Network::TcpSocketChannel>>;

      /**
       * Constructs a RiskWebServlet.
       * @param sessions The available web sessions.
       * @param clients The clients used to access Spire services.
       */
      RiskWebServlet(
        Beam::Ref<Beam::WebServices::SessionStore<WebPortalSession>> sessions,
        Clients clients);

      ~RiskWebServlet();

      std::vector<Beam::WebServices::HttpRequestSlot> get_slots();

      std::vector<Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>>
        get_web_socket_slots();

      void close();

    private:
      using StompServer =
        Beam::Stomp::StompServer<std::unique_ptr<WebSocketChannel>>;
      struct PortfolioFilter {
        std::unordered_set<Beam::ServiceLocator::DirectoryEntry> m_groups;
        std::unordered_set<Venue> m_venues;
        std::unordered_set<CurrencyId> m_currencies;

        bool is_filtered(const PortfolioModel::Entry& entry,
          const Beam::ServiceLocator::DirectoryEntry& group) const;
      };
      struct PortfolioSubscriber {
        Beam::ServiceLocator::DirectoryEntry m_account;
        StompServer m_client;
        std::string m_subscription_id;
        PortfolioFilter m_filter;

        PortfolioSubscriber(Beam::ServiceLocator::DirectoryEntry account,
          std::unique_ptr<WebSocketChannel> channel);
      };
      Clients m_clients;
      Beam::WebServices::SessionStore<WebPortalSession>* m_sessions;
      std::unordered_map<RiskPortfolioKey, PortfolioModel::Entry>
        m_portfolio_entries;
      std::unordered_set<PortfolioModel::Entry> m_updated_portfolio_entries;
      std::vector<std::shared_ptr<PortfolioSubscriber>> m_portfolio_subscribers;
      PortfolioModel m_portfolio_model;
      Beam::Threading::TimerBox m_portfolio_timer;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry> m_trader_groups;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      RiskWebServlet(const RiskWebServlet&) = delete;
      RiskWebServlet& operator=(const RiskWebServlet&) = delete;
      const Beam::ServiceLocator::DirectoryEntry& find_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& trader);
      void send_portfolio_entry(const PortfolioModel::Entry& entry,
        const Beam::ServiceLocator::DirectoryEntry& group,
        PortfolioSubscriber& subscriber, bool check_filter);
      void on_portfolio_upgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
      void on_portfolio_request(
        const std::shared_ptr<PortfolioSubscriber>& subscriber,
        const Beam::Stomp::StompFrame& frame);
      void on_portfolio_filter_request(
        const std::shared_ptr<PortfolioSubscriber>& subscriber,
        const Beam::Stomp::StompFrame& frame);
      void on_portfolio_update(const PortfolioModel::Entry& entry);
      void on_portfolio_timer_expired(Beam::Threading::Timer::Result result);
  };
}

#endif
