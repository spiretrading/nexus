#ifndef NEXUS_RISK_SERVLET_HPP
#define NEXUS_RISK_SERVLET_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskServices.hpp"
#include "Nexus/RiskService/RiskSession.hpp"

namespace Nexus::RiskService {

  /**
   * Monitors a trader's positions and orders for risk compliance.
   * @param <C> The container instantiating this servlet.
   * @param <A> The type of AdministrationClient to use.
   * @param <M> The type of MarketDataClient to use.
   * @param <O> The type of OrderExecutionClient to use.
   * @param <R> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <T> The type of TimeClient to use.
   * @param <D> The type of RiskDataStore to use.
   */
  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  class RiskServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * The type of AdministrationClient to use used.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = R;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of RiskDataStore to use. */
      using RiskDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a RiskServlet.
       * @param accounts Publishes the accounts whose risk is to be managed.
       * @param administrationClient Initializes the AdministrationClient.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orderExecutionClient Initializes the OrderExecutionClient.
       * @param transitionTimer Initializes the transition Timer.
       * @param timeClient Initializes the TimeClient.
       * @param dataStore Initializes the RiskDataStore.
       * @param exchangeRates The list of exchange rates.
       * @param markets The market database used by the portfolio.
       * @param destinations The destination database used to flatten positions.
       */
      template<typename AF, typename MF, typename OF, typename TF, typename DF>
      RiskServlet(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, std::function<
        std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
        TF&& timeClient, DF&& dataStore,
        std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
        DestinationDatabase destinations);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      using ConsolidatedRiskController =
        RiskService::ConsolidatedRiskController<AdministrationClient*, M, O, R,
        T, D>;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry, Beam::Threading::Mutex>
        m_accountToGroup;
      Beam::SynchronizedVector<ServiceProtocolClient*, Beam::Threading::Mutex>
        m_portfolioSubscribers;
      boost::optional<ConsolidatedRiskController> m_controller;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::ServiceLocator::DirectoryEntry LoadGroup(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnSubscribeRiskPortfolioUpdatesRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, SubscribeRiskPortfolioUpdatesService>& request);
  };

  template<typename A, typename M, typename O, typename R, typename T,
    typename D>
  struct MetaRiskServlet {
    using Session = RiskSession;
    template<typename C>
    struct apply {
      using type = RiskServlet<C, A, M, O, R, T, D>;
    };
  };

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  template<typename AF, typename MF, typename OF, typename TF, typename DF>
  RiskServlet<C, A, M, O, R, T, D>::RiskServlet(
    Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
    AF&& administrationClient, MF&& marketDataClient,
    OF&& orderExecutionClient, std::function<
    std::unique_ptr<TransitionTimer> ()> transitionTimerFactory,
    TF&& timeClient, DF&& dataStore,
    std::vector<ExchangeRate> exchangeRates, MarketDatabase markets,
    DestinationDatabase destinations) {}

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRiskServices(Store(slots));
    RegisterRiskMessages(Store(slots));
    SubscribeRiskPortfolioUpdatesService::AddRequestSlot(Store(slots),
      std::bind(&RiskServlet::OnSubscribeRiskPortfolioUpdatesRequest, this,
      std::placeholders::_1));
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_portfolioSubscribers.Remove(&client);
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::Shutdown() {
    m_openState.SetClosed();
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  Beam::ServiceLocator::DirectoryEntry RiskServlet<C, A, M, O, R, T, D>::
      LoadGroup(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_accountToGroup.GetOrInsert(account,
      [&] {
        return m_administrationClient->LoadParentTradingGroup(account);
      });
  }

  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D>
  void RiskServlet<C, A, M, O, R, T, D>::OnSubscribeRiskPortfolioUpdatesRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      SubscribeRiskPortfolioUpdatesService>& request) {
    auto& session = request.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    auto groups = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    if(!isAdministrator) {
      groups = m_administrationClient->LoadManagedTradingGroups(
        session.GetAccount());
    }
    auto entries = std::vector<RiskInventoryEntry>();
    m_portfolioSubscribers.With([&] (auto& subscribers) {
      if(isAdministrator) {
        session.AddAllPortfolioGroups();
      } else {
        for(auto& group : groups) {
          session.AddPortfolioGroup(group);
        }
      }
      subscribers.push_back(&request.GetClient());
      auto queue = std::make_shared<Beam::Queue<RiskInventoryEntry>>();
      m_controller->GetPortfolioPublisher().Monitor(queue);
      while(auto entry = queue->TryPop()) {
        auto group = LoadGroup(entry->m_key.m_account);
        if(session.HasGroupPortfolioSubscription(group)) {
          entries.push_back(std::move(*entry));
        }
      }
      request.SetResult(entries);
    });
  }
}

#endif
