#ifndef NEXUS_RISKSTATEMONITOR_HPP
#define NEXUS_RISKSTATEMONITOR_HPP
#include <unordered_map>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/FilteredQueueWriter.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueReader.hpp>
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

namespace Nexus {
namespace RiskService {

  //! Represents an entry in a RiskState table.
  using RiskStateEntry = Beam::KeyValuePair<
    Beam::ServiceLocator::DirectoryEntry, RiskState>;

  /*! \class RiskStateMonitor
      \brief Instantiates RiskStateTrackers for all accounts executing Orders
             and notifies when changes to an account's RiskState occur.
             Portfolio's are valued using the BboQuote published by a
             MarketDataClient and Orders published by an
             OrderExecutionPublisher.
      \tparam RiskStateTrackerType The type of RiskStateTracker to update.
      \tparam AdministrationClientType The type of AdministrationClient used to
              load an account's RiskParameters.
      \tparam MarketDataClientType The type of MarketDataClient to use.
      \tparam TransitionTimerType The type of Timer to use to transition from
              CLOSED_ORDERS to DISABLED.
      \tparam TimeClientType The type of TimeClient to use.
   */
  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  class RiskStateMonitor : private boost::noncopyable {
    public:

      //! The type of RiskStateTrackers to update.
      using RiskStateTracker =
        Beam::GetTryDereferenceType<RiskStateTrackerType>;

      //! The type of AdministrationClient used to load an account's
      //! RiskParameters.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! The type of MarketDataClient to use.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! The type of TransitionTimer to use.
      using TransitionTimer = Beam::GetTryDereferenceType<TransitionTimerType>;

      //! The type of TimeClient to use.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a RiskStateMonitor.
      /*!
        \param administrationClient Initializes the AdministrationClient.
        \param marketDataClient Initializes the MarketDataClient.
        \param orderSubmissionQueue Used to monitor Order submissions.
        \param transitionTimer Initializes the transition Timer.
        \param timeClient Initializes the TimeClient.
        \param marketDatabase The database of markets.
        \param exchangeRates The list of ExchangeRates.
      */
      template<typename AdministrationClientForward,
        typename MarketDataClientForward, typename TransitionTimerForward,
        typename TimeClientForward>
      RiskStateMonitor(AdministrationClientForward&& administrationClient,
        MarketDataClientForward&& marketDataClient,
        Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders,
        TransitionTimerForward&& transitionTimer,
        TimeClientForward&& timeClient, const MarketDatabase& marketDatabase,
        std::vector<ExchangeRate> exchangeRates);

      ~RiskStateMonitor();

      //! Returns the object used to publish RiskState updates.
      const Beam::Publisher<RiskStateEntry>& GetRiskStatePublisher() const;

      //! Returns the object used to publish Inventory updates.
      const RiskPortfolioUpdatePublisher& GetInventoryPublisher() const;

    private:
      using PortfolioMonitor = Accounting::PortfolioMonitor<
        typename RiskStateTracker::Portfolio*, MarketDataClient*>;
      struct AccountEntry : private boost::noncopyable {
        RiskStateTracker m_tracker;
        PortfolioMonitor m_portfolio;

        AccountEntry(const Beam::ServiceLocator::DirectoryEntry& account,
          const MarketDatabase& marketDatabase,
          const std::vector<ExchangeRate>& exchangeRates,
          Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders,
          MarketDataClient* marketDataClient, TimeClient* timeClient);
      };
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      std::shared_ptr<Beam::Publisher<const OrderExecutionService::Order*>>
        m_orderPublisher;
      Beam::GetOptionalLocalPtr<TransitionTimerType> m_transitionTimer;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
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

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::AccountEntry::
      AccountEntry(const Beam::ServiceLocator::DirectoryEntry& account,
      const MarketDatabase& marketDatabase,
      const std::vector<ExchangeRate>& exchangeRates,
      Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders,
      MarketDataClient* marketDataClient, TimeClient* timeClient)
      : m_tracker(typename RiskStateTracker::Portfolio(marketDatabase),
          exchangeRates, timeClient),
        m_portfolio(&m_tracker.GetPortfolio(), marketDataClient,
          std::move(orders)) {}

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  template<typename AdministrationClientForward,
    typename MarketDataClientForward, typename TransitionTimerForward,
    typename TimeClientForward>
  RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      RiskStateMonitor(AdministrationClientForward&& administrationClient,
      MarketDataClientForward&& marketDataClient,
      Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders,
      TransitionTimerForward&& transitionTimer, TimeClientForward&& timeClient,
      const MarketDatabase& marketDatabase,
      std::vector<ExchangeRate> exchangeRates)
      : m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)),
        m_marketDataClient(std::forward<MarketDataClientForward>(
          marketDataClient)),
        m_orderPublisher(Beam::MakeSequencePublisherAdaptor(
          std::make_shared<Beam::QueueReaderPublisher<
          const OrderExecutionService::Order*>>(std::move(orders)))),
        m_transitionTimer(std::forward<TransitionTimerForward>(
          transitionTimer)),
        m_timeClient(std::forward<TimeClientForward>(timeClient)),
        m_marketDatabase(marketDatabase),
        m_exchangeRates(std::move(exchangeRates)) {
    m_orderPublisher->Monitor(
      m_tasks.GetSlot<const OrderExecutionService::Order*>(std::bind(
      &RiskStateMonitor::OnOrderSubmission, this, std::placeholders::_1)));
    m_transitionTimer->GetPublisher().Monitor(
      m_tasks.GetSlot<Beam::Threading::Timer::Result>(
      std::bind(&RiskStateMonitor::OnTransitionTimer, this,
      std::placeholders::_1)));
    m_transitionTimer->Start();
  }

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      ~RiskStateMonitor() {
    m_tasks.Break();
  }

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  const Beam::Publisher<RiskStateEntry>&
      RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      GetRiskStatePublisher() const {
    return m_riskPublisher;
  }

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  const RiskPortfolioUpdatePublisher& RiskStateMonitor<RiskStateTrackerType,
      AdministrationClientType, MarketDataClientType, TransitionTimerType,
      TimeClientType>::GetInventoryPublisher() const {
    return m_inventoryPublisher;
  }

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  void RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      OnTransitionTimer(const Beam::Threading::Timer::Result& result) {
    for(const auto& accountEntry : m_accountEntries) {
      const auto& account = accountEntry.first;
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

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  void RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      OnOrderSubmission(const OrderExecutionService::Order* order) {
    const auto& account = order->GetInfo().m_fields.m_account;
    auto accountIterator = m_accountEntries.find(account);
    if(accountIterator != m_accountEntries.end()) {
      return;
    }
    m_administrationClient->GetRiskParametersPublisher(account).Monitor(
      m_tasks.GetSlot<RiskParameters>(
      std::bind(&RiskStateMonitor::OnRiskParametersModified, this, account,
      std::placeholders::_1)));
    auto accountOrders = std::make_shared<
      Beam::Queue<const OrderExecutionService::Order*>>();
    m_orderPublisher->Monitor(Beam::MakeFilteredQueueWriter(accountOrders,
      [=] (auto order) {
        return order->GetInfo().m_fields.m_account == account;
      }));
    auto& entry = *m_accountEntries.emplace(account,
      std::make_shared<AccountEntry>(account, m_marketDatabase, m_exchangeRates,
      accountOrders, &*m_marketDataClient, &*m_timeClient)).first->second;
    entry.m_portfolio.GetPublisher().Monitor(
      m_tasks.GetSlot<typename PortfolioMonitor::UpdateEntry>(
      std::bind(&RiskStateMonitor::OnPortfolioUpdate, this, account,
      std::placeholders::_1)));
  }

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  void RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      OnRiskParametersModified(
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

  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType>
  void RiskStateMonitor<RiskStateTrackerType, AdministrationClientType,
      MarketDataClientType, TransitionTimerType, TimeClientType>::
      OnPortfolioUpdate(const Beam::ServiceLocator::DirectoryEntry& account,
      const typename PortfolioMonitor::UpdateEntry& update) {
    auto accountIterator = m_accountEntries.find(account);
    if(accountIterator == m_accountEntries.end()) {
      return;
    }
    auto& entry = *accountIterator->second;
    RiskPortfolioKey key(account,
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
}

#endif
