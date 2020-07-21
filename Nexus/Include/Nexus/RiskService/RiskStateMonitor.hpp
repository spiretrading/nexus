#ifndef NEXUS_RISK_STATE_MONITOR_HPP
#define NEXUS_RISK_STATE_MONITOR_HPP
#include <unordered_map>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/FilteredPublisher.hpp>
#include <Beam/Queues/QueuePublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Timer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Accounting/PortfolioMonitor.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskStateTracker.hpp"

namespace Nexus::RiskService {

  /** Represents an entry in a RiskState table. */
  using RiskStateEntry = Beam::TableEntry<Beam::ServiceLocator::DirectoryEntry,
    RiskState>;

  /**
   * Instantiates RiskStateTrackers for all accounts executing Orders and
   * notifies when changes to an account's RiskState occur.
   * Portfolio's are valued using the BboQuote published by a MarketDataClient
   * and Orders published by an OrderExecutionPublisher.
   * @param <R> The type of RiskStateTracker to update.
   * @param <A> The type of AdministrationClient used to load an account's
   *        RiskParameters.
   * @param <M> The type of MarketDataClient to use.
   * @param <T> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <C> The type of TimeClient to use.
   */
  template<typename R, typename A, typename M, typename T, typename C>
  class RiskStateMonitor : private boost::noncopyable {
    public:

      /** The type of RiskStateTrackers to update. */
      using RiskStateTracker = Beam::GetTryDereferenceType<R>;

      /**
       * The type of AdministrationClient used to load an account's
       * RiskParameters.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = Beam::GetTryDereferenceType<T>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a RiskStateMonitor.
       * @param administrationClient Initializes the AdministrationClient.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orderSubmissionQueue Used to monitor Order submissions.
       * @param transitionTimer Initializes the transition Timer.
       * @param timeClient Initializes the TimeClient.
       * @param marketDatabase The database of markets.
       * @param exchangeRates The list of ExchangeRates.
       */
      template<typename AF, typename MF, typename TF, typename CF>
      RiskStateMonitor(AF&& administrationClient, MF&& marketDataClient,
        const std::shared_ptr<
        Beam::QueueReader<const OrderExecutionService::Order*>>&
        orderSubmissionQueue, TF&& transitionTimer, CF&& timeClient,
        const MarketDatabase& marketDatabase,
        std::vector<ExchangeRate> exchangeRates);

      /** Returns the object used to publish RiskState updates. */
      const Beam::Publisher<RiskStateEntry>& GetRiskStatePublisher() const;

      /** Returns the object used to publish Inventory updates. */
      const RiskPortfolioUpdatePublisher& GetInventoryPublisher() const;

    private:
      using PortfolioMonitor = Accounting::PortfolioMonitor<
        typename RiskStateTracker::Portfolio*, MarketDataClient*>;
      struct AccountEntry : private boost::noncopyable {
        std::shared_ptr<Beam::FilteredPublisher<Beam::SequencePublisher<
          const OrderExecutionService::Order*>>> m_orderPublisher;
        RiskStateTracker m_tracker;
        PortfolioMonitor m_portfolio;

        AccountEntry(const Beam::ServiceLocator::DirectoryEntry& account,
          const MarketDatabase& marketDatabase,
          const std::vector<ExchangeRate>& exchangeRates,
          const Beam::Publisher<const OrderExecutionService::Order*>&
          orderPublisher, MarketDataClient* marketDataClient,
          TimeClient* timeClient);
      };
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<M> m_marketDataClient;
      Beam::QueuePublisher<Beam::SequencePublisher<
        const OrderExecutionService::Order*>> m_orderSubmissionPublisher;
      Beam::GetOptionalLocalPtr<T> m_transitionTimer;
      Beam::GetOptionalLocalPtr<C> m_timeClient;
      MarketDatabase m_marketDatabase;
      std::vector<ExchangeRate> m_exchangeRates;
      Beam::TablePublisher<Beam::ServiceLocator::DirectoryEntry, RiskState>
        m_riskPublisher;
      Beam::TablePublisher<RiskPortfolioKey, RiskPortfolioInventory>
        m_inventoryPublisher;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<AccountEntry>> m_accountEntries;
      Beam::RoutineTaskQueue m_tasks;

      void OnTransitionTimer(const Beam::Threading::Timer::Result& result);
      void OnOrderSubmission(const OrderExecutionService::Order* order);
      void OnRiskParametersModified(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskParameters& parameters);
      void OnPortfolioUpdate(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const typename PortfolioMonitor::UpdateEntry& update);
  };

  template<typename R, typename A, typename M, typename T, typename C>
  RiskStateMonitor<R, A, M, T, C>::AccountEntry::AccountEntry(
    const Beam::ServiceLocator::DirectoryEntry& account,
    const MarketDatabase& marketDatabase,
    const std::vector<ExchangeRate>& exchangeRates,
    const Beam::Publisher<const OrderExecutionService::Order*>& orderPublisher,
    MarketDataClient* marketDataClient, TimeClient* timeClient)
      : m_orderPublisher(std::make_shared<Beam::FilteredPublisher<
          Beam::SequencePublisher<const OrderExecutionService::Order*>>>(
          [=] (auto order) {
            return order->GetInfo().m_fields.m_account == account;
          }, Beam::Initialize())),
        m_tracker(typename RiskStateTracker::Portfolio(marketDatabase),
          exchangeRates, timeClient),
        m_portfolio(&m_tracker.GetPortfolio(), marketDataClient,
          *m_orderPublisher) {
    orderPublisher.Monitor(m_orderPublisher);
  }

  template<typename R, typename A, typename M, typename T, typename C>
  template<typename AF, typename MF, typename TF, typename CF>
  RiskStateMonitor<R, A, M, T, C>::RiskStateMonitor(AF&& administrationClient,
      MF&& marketDataClient, const std::shared_ptr<
      Beam::QueueReader<const OrderExecutionService::Order*>>&
      orderSubmissionQueue, TF&& transitionTimer, CF&& timeClient,
      const MarketDatabase& marketDatabase,
      std::vector<ExchangeRate> exchangeRates)
      : m_administrationClient(std::forward<AF>(administrationClient)),
        m_marketDataClient(std::forward<MF>(marketDataClient)),
        m_orderSubmissionPublisher(orderSubmissionQueue),
        m_transitionTimer(std::forward<TF>(transitionTimer)),
        m_timeClient(std::forward<CF>(timeClient)),
        m_marketDatabase(marketDatabase),
        m_exchangeRates(std::move(exchangeRates)) {
    m_orderSubmissionPublisher.Monitor(
      m_tasks.GetSlot<const OrderExecutionService::Order*>(std::bind(
      &RiskStateMonitor::OnOrderSubmission, this, std::placeholders::_1)));
    m_transitionTimer->GetPublisher().Monitor(
      m_tasks.GetSlot<Beam::Threading::Timer::Result>(
      std::bind(&RiskStateMonitor::OnTransitionTimer, this,
      std::placeholders::_1)));
    m_transitionTimer->Start();
  }

  template<typename R, typename A, typename M, typename T, typename C>
  const Beam::Publisher<RiskStateEntry>& RiskStateMonitor<R, A, M, T, C>::
      GetRiskStatePublisher() const {
    return m_riskPublisher;
  }

  template<typename R, typename A, typename M, typename T, typename C>
  const RiskPortfolioUpdatePublisher& RiskStateMonitor<R, A, M, T, C>::
      GetInventoryPublisher() const {
    return m_inventoryPublisher;
  }

  template<typename R, typename A, typename M, typename T, typename C>
  void RiskStateMonitor<R, A, M, T, C>::OnTransitionTimer(
      const Beam::Threading::Timer::Result& result) {
    for(auto& accountEntry : m_accountEntries) {
      auto& account = accountEntry.first;
      auto& entry = *accountEntry.second;
      auto previousState = entry.m_tracker.GetRiskState();
      entry.m_tracker.Update(boost::posix_time::seconds(1));
      auto currentState = entry.m_tracker.GetRiskState();
      if(previousState != currentState) {
        m_riskPublisher.Push(RiskStateEntry(account, currentState));
      }
    }
    m_transitionTimer->Start();
  }

  template<typename R, typename A, typename M, typename T, typename C>
  void RiskStateMonitor<R, A, M, T, C>::OnOrderSubmission(
      const OrderExecutionService::Order* order) {
    auto& account = order->GetInfo().m_fields.m_account;
    auto accountIterator = m_accountEntries.find(account);
    if(accountIterator != m_accountEntries.end()) {
      return;
    }
    m_administrationClient->GetRiskParametersPublisher(account).Monitor(
      m_tasks.GetSlot<RiskParameters>(
      std::bind(&RiskStateMonitor::OnRiskParametersModified, this, account,
      std::placeholders::_1)));
    auto& entry = *m_accountEntries.emplace(account,
      std::make_shared<AccountEntry>(account, m_marketDatabase, m_exchangeRates,
      m_orderSubmissionPublisher, &*m_marketDataClient,
      &*m_timeClient)).first->second;
    entry.m_portfolio.GetPublisher().Monitor(
      m_tasks.GetSlot<typename PortfolioMonitor::UpdateEntry>(
      std::bind(&RiskStateMonitor::OnPortfolioUpdate, this, account,
      std::placeholders::_1)));
  }

  template<typename R, typename A, typename M, typename T, typename C>
  void RiskStateMonitor<R, A, M, T, C>::OnRiskParametersModified(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskParameters& riskParameters) {
    auto accountIterator = m_accountEntries.find(account);
    if(accountIterator == m_accountEntries.end()) {
      return;
    }
    auto& entry = *accountIterator->second;
    auto previousState = entry.m_tracker.GetRiskState();
    entry.m_tracker.Update(riskParameters);
    auto currentState = entry.m_tracker.GetRiskState();
    if(previousState != currentState) {
      m_riskPublisher.Push(RiskStateEntry(account, currentState));
    }
  }

  template<typename R, typename A, typename M, typename T, typename C>
  void RiskStateMonitor<R, A, M, T, C>::OnPortfolioUpdate(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const typename PortfolioMonitor::UpdateEntry& update) {
    auto accountIterator = m_accountEntries.find(account);
    if(accountIterator == m_accountEntries.end()) {
      return;
    }
    auto& entry = *accountIterator->second;
    auto key = RiskPortfolioKey(account,
      update.m_securityInventory.m_position.m_key.m_index);
    m_inventoryPublisher.Push(key, update.m_securityInventory);
    auto previousState = entry.m_tracker.GetRiskState();
    entry.m_tracker.Update();
    auto currentState = entry.m_tracker.GetRiskState();
    if(previousState != currentState) {
      m_riskPublisher.Push(RiskStateEntry(account, currentState));
    }
  }
}

#endif
