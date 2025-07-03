#ifndef NEXUS_PORTFOLIO_CONTROLLER_HPP
#define NEXUS_PORTFOLIO_CONTROLLER_HPP
#include <unordered_map>
#include <unordered_set>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Queues/ValueSnapshotPublisher.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Accounting {

  /**
   * Uses the BboQuote published by a MarketDataClient and the Orders
   * published by an OrderExecutionPublisher to update a Portfolio.
   * @param <P> The type of Portfolio to update.
   * @param <C> The type of MarketDataClient to use.
   */
  template<typename P, typename C>
  class PortfolioController {
    public:

      /** The type of Portfolio to update. */
      using Portfolio = Beam::GetTryDereferenceType<P>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /** The type of Inventory stored by the Portfolio. */
      using Inventory = typename Portfolio::Bookkeeper::Inventory;

      /** The type of updates published. */
      using UpdateEntry = typename Portfolio::UpdateEntry;

      /**
       * Constructs a PortfolioController.
       * @param portfolio Initializes the Portfolio.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orders The Orders to include in the Portfolio.
       */
      template<typename PF, typename CF>
      PortfolioController(PF&& portfolio, CF&& marketDataClient,
        Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders);

      /** Returns the object publishing updates to the Portfolio. */
      const Beam::SnapshotPublisher<UpdateEntry, Portfolio*>&
        GetPublisher() const;

    private:
      Beam::GetOptionalLocalPtr<P> m_portfolio;
      Beam::GetOptionalLocalPtr<C> m_marketDataClient;
      OrderExecutionService::ExecutionReportPublisher
        m_executionReportPublisher;
      Beam::ValueSnapshotPublisher<UpdateEntry, Portfolio*> m_publisher;
      std::unordered_map<Security, BboQuote> m_bboQuotes;
      std::unordered_set<Security> m_securities;
      Beam::RoutineTaskQueue m_tasks;

      PortfolioController(const PortfolioController&) = delete;
      PortfolioController& operator =(const PortfolioController&) = delete;
      void Subscribe(const Security& security);
      void PushUpdate(const Security& security);
      void OnBbo(const Security& security, const BboQuote& bbo);
      void OnExecutionReport(
        const OrderExecutionService::ExecutionReportEntry& executionReport);
  };

  template<typename P, typename C>
  template<typename PF, typename CF>
  PortfolioController<P, C>::PortfolioController(PF&& portfolio,
      CF&& marketDataClient,
      Beam::ScopedQueueReader<const OrderExecutionService::Order*> orders)
      : m_portfolio(std::forward<PF>(portfolio)),
        m_marketDataClient(std::forward<CF>(marketDataClient)),
        m_executionReportPublisher(std::move(orders)),
        m_publisher([] (auto snapshot, auto& queue) {
          ForEach(*snapshot, [&] (const auto& update) {
            queue.Push(update);
          });
        }, Beam::SignalHandling::NullSlot(), &*m_portfolio) {
    m_publisher.With([&] {
      for(auto& inventory : m_portfolio->GetBookkeeper().GetInventoryRange()) {
        Subscribe(inventory.m_position.m_key.m_index);
      }
      auto snapshot = boost::optional<
        std::vector<OrderExecutionService::ExecutionReportEntry>>();
      m_executionReportPublisher.Monitor(
        m_tasks.GetSlot<OrderExecutionService::ExecutionReportEntry>(
          std::bind_front(&PortfolioController::OnExecutionReport, this)),
          Beam::Store(snapshot));
      if(snapshot) {
        for(auto& report : *snapshot) {
          OnExecutionReport(report);
        }
      }
    });
  }

  template<typename P, typename C>
  const Beam::SnapshotPublisher<typename PortfolioController<P, C>::UpdateEntry,
      typename PortfolioController<P, C>::Portfolio*>&
      PortfolioController<P, C>::GetPublisher() const {
    return m_publisher;
  }

  template<typename P, typename C>
  void PortfolioController<P, C>::Subscribe(const Security& security) {
    if(auto securityIterator = m_securities.find(security);
        securityIterator == m_securities.end()) {
      auto snapshot = std::make_shared<Beam::StateQueue<BboQuote>>();
      m_marketDataClient->QueryBboQuotes(
        Beam::Queries::MakeLatestQuery(security), snapshot);
      try {
        OnBbo(security, snapshot->Pop());
      } catch(const std::exception&) {
      }
      m_marketDataClient->QueryBboQuotes(Beam::Queries::MakeRealTimeQuery(
        security), m_tasks.GetSlot<BboQuote>(std::bind_front(
          &PortfolioController::OnBbo, this, security)));
      m_securities.insert(security);
    }
  }

  template<typename P, typename C>
  void PortfolioController<P, C>::PushUpdate(const Security& security) {
    auto securityEntryIterator = m_portfolio->GetSecurityEntries().find(
      security);
    if(securityEntryIterator == m_portfolio->GetSecurityEntries().end()) {
      return;
    }
    auto& securityEntry = securityEntryIterator->second;
    auto& securityInventory = m_portfolio->GetBookkeeper().GetInventory(
      security, securityEntry.m_valuation.m_currency);
    auto update = UpdateEntry();
    update.m_securityInventory = securityInventory;
    update.m_unrealizedSecurity = securityEntry.m_unrealized;
    update.m_currencyInventory = m_portfolio->GetBookkeeper().GetTotal(
      securityEntry.m_valuation.m_currency);
    auto unrealizedCurrencyIterator =
      m_portfolio->GetUnrealizedProfitAndLosses().find(
        securityEntry.m_valuation.m_currency);
    if(unrealizedCurrencyIterator ==
        m_portfolio->GetUnrealizedProfitAndLosses().end()) {
      update.m_unrealizedCurrency = Money::ZERO;
    } else {
      update.m_unrealizedCurrency = unrealizedCurrencyIterator->second;
    }
    m_publisher.Push(update);
  }

  template<typename P, typename C>
  void PortfolioController<P, C>::OnBbo(const Security& security,
      const BboQuote& bbo) {
    auto& lastBbo = m_bboQuotes[security];
    if(lastBbo.m_ask.m_price == bbo.m_ask.m_price &&
        lastBbo.m_bid.m_price == bbo.m_bid.m_price) {
      return;
    }
    lastBbo = bbo;
    if(bbo.m_ask.m_price == Money::ZERO && bbo.m_bid.m_price == Money::ZERO) {
      return;
    }
    m_publisher.With([&] {
      auto hasUpdate = [&] {
        if(bbo.m_ask.m_price == Money::ZERO) {
          return m_portfolio->UpdateBid(security, bbo.m_bid.m_price);
        } else if(bbo.m_bid.m_price == Money::ZERO) {
          return m_portfolio->UpdateAsk(security, bbo.m_ask.m_price);
        }
        return m_portfolio->Update(security, bbo.m_ask.m_price,
          bbo.m_bid.m_price);
      }();
      if(hasUpdate) {
        PushUpdate(security);
      }
    });
  }

  template<typename P, typename C>
  void PortfolioController<P, C>::OnExecutionReport(
      const OrderExecutionService::ExecutionReportEntry& executionReport) {
    if(executionReport.m_executionReport.m_status == OrderStatus::PENDING_NEW) {
      Subscribe(executionReport.m_order->GetInfo().m_fields.m_security);
    }
    m_publisher.With([&] {
      if(m_portfolio->Update(executionReport.m_order->GetInfo().m_fields,
          executionReport.m_executionReport)) {
        PushUpdate(executionReport.m_order->GetInfo().m_fields.m_security);
      }
    });
  }
}

#endif
