#ifndef NEXUS_RISK_CONTROLLER_HPP
#define NEXUS_RISK_CONTROLLER_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/QueuePipe.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskStateProcessor.hpp"
#include "Nexus/RiskService/RiskTransitionProcessor.hpp"

namespace Nexus::RiskService {

  /** Represents an entry in a RiskState table. */
  using RiskStateEntry = Beam::KeyValuePair<
    Beam::ServiceLocator::DirectoryEntry, RiskState>;

  /**
   * Instantiates RiskStateProcessors and RiskTransitionProcessors for all
   * accounts received from a queue and updates both processors based on Orders
   * submitted and market data. Portfolio's are valued using the BboQuote
   * published by a MarketDataClient and Orders published by an
   * OrderExecutionPublisher.
   * @param <A> The type of AdministrationClient used to load an account's
   *        RiskParameters.
   * @param <M> The type of MarketDataClient to use.
   * @param <O> The type of OrderExecutionClient to use.
   * @param <R> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <T> The type of TimeClient to use.
   */
  template<typename A, typename M, typename O, typename R, typename T>
  class RiskController : private boost::noncopyable {
    public:

      /**
       * The type of AdministrationClient used to load an account's
       * RiskParameters.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = Beam::GetTryDereferenceType<R>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a RiskController.
       * @param accounts The QueueReader publishing the accounts to control.
       * @param administrationClient Initializes the AdministrationClient.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orderExecutionClient Initializes the OrderExecutionClient.
       * @param transitionTimer Initializes the transition Timer.
       * @param timeClient Initializes the TimeClient.
       * @param exchangeRates The list of exchange rates.
       * @param destinations The destination database used to flatten positions.
       */
      template<typename AF, typename MF, typename OF, typename RF, typename TF>
      RiskController(
        Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
        AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
        const std::vector<ExchangeRate>& exchangeRates,
        DestinationDatabase destinations);

      /** Returns the object used to publish RiskState updates. */
      const Beam::Publisher<RiskStateEntry>& GetRiskStatePublisher() const;

      /** Returns the object used to publish Inventory updates. */
      const RiskPortfolioUpdatePublisher& GetInventoryPublisher() const;

    private:
      struct AccountEntry {
      };
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<M> m_marketDataClient;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      Beam::GetOptionalLocalPtr<R> m_transitionTimer;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      DestinationDatabase m_destinations;
      Beam::TablePublisher<Beam::ServiceLocator::DirectoryEntry, RiskState>
        m_riskStatePublisher;
      Beam::TablePublisher<RiskPortfolioKey, RiskPortfolioInventory>
        m_inventoryPublisher;
      boost::optional<Beam::QueuePipe<Beam::ServiceLocator::DirectoryEntry>>
        m_accountPipe;
      Beam::RoutineTaskQueue m_tasks;

      void OnAccountAdded(const Beam::ServiceLocator::DirectoryEntry& account);
      void OnTransitionTimer(Beam::Threading::Timer::Result result);
      void OnRiskParametersUpdate(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskParameters& riskParameters);
  };

  template<typename A, typename M, typename O, typename R, typename T>
  RiskController(Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry>,
    A&&, M&&, O&&, R&&, T&&, const std::vector<ExchangeRate>&,
    DestinationDatabase) -> RiskController<std::remove_reference_t<A>,
    std::remove_reference_t<M>, std::remove_reference_t<O>,
    std::remove_reference_t<R>, std::remove_reference_t<T>>;

  template<typename A, typename M, typename O, typename R, typename T>
  template<typename AF, typename MF, typename OF, typename RF, typename TF>
  RiskController<A, M, O, R, T>::RiskController(
      Beam::ScopedQueueReader<Beam::ServiceLocator::DirectoryEntry> accounts,
      AF&& administrationClient, MF&& marketDataClient,
      OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
      const std::vector<ExchangeRate>& exchangeRates,
      DestinationDatabase destinations)
      : m_administrationClient(std::forward<AF>(administrationClient)),
        m_marketDataClient(std::forward<MF>(marketDataClient)),
        m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
        m_transitionTimer(std::forward<RF>(transitionTimer)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_destinations(std::move(destinations)) {
    m_accountPipe.emplace(std::move(accounts),
      m_tasks.GetSlot<Beam::ServiceLocator::DirectoryEntry>(
      std::bind(&RiskController::OnAccountAdded, this,
      std::placeholders::_1)));
  }

  template<typename A, typename M, typename O, typename R, typename T>
  const Beam::Publisher<RiskStateEntry>&
      RiskController<A, M, O, R, T>::GetRiskStatePublisher() const {
    return m_riskStatePublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T>
  const RiskPortfolioUpdatePublisher&
      RiskController<A, M, O, R, T>::GetInventoryPublisher() const {
    return m_inventoryPublisher;
  }

  template<typename A, typename M, typename O, typename R, typename T>
  void RiskController<A, M, O, R, T>::OnAccountAdded(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    m_administrationClient->GetRiskParametersPublisher(account).Monitor(
      m_tasks.GetSlot<RiskParameters>(
      std::bind(&RiskController::OnRiskParametersUpdate, this, account,
      std::placeholders::_1)));
/*
    auto orders = std::make_shared<
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
*/
  }

  template<typename A, typename M, typename O, typename R, typename T>
  void RiskController<A, M, O, R, T>::OnTransitionTimer(
      Beam::Threading::Timer::Result result) {
/*
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
*/
    m_transitionTimer->Start();
  }

  template<typename A, typename M, typename O, typename R, typename T>
  void RiskController<A, M, O, R, T>::OnRiskParametersUpdate(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskParameters& riskParameters) {
/*
    auto& entry = *accountIterator->second;
    auto previousState = entry.m_tracker.GetRiskState();
    entry.m_tracker.Update(riskParameters);
    auto currentState = entry.m_tracker.GetRiskState();
    if(previousState != currentState) {
      m_riskPublisher.Push(RiskStateEntry(account, currentState));
    }
*/
  }
}

#endif
