#ifndef NEXUS_PORTFOLIO_MONITOR_HPP
#define NEXUS_PORTFOLIO_MONITOR_HPP
#include <unordered_set>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/NativePointerPolicy.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ValueSnapshotPublisher.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Accounting {

  /** Uses the BboQuote published by a MarketDataClient and the Orders
      published by an OrderExecutionPublisher to update a Portfolio.
      \tparam PortfolioType The type of Portfolio to update.
      \tparam MarketDataClientType The type of MarketDataClient to use.
   */
  template<typename PortfolioType, typename MarketDataClientType>
  class PortfolioMonitor : private boost::noncopyable {
    public:

      //! The type of Portfolio to update.
      using Portfolio = Beam::GetTryDereferenceType<PortfolioType>;

      //! The type of MarketDataClient to use.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! The type of Inventory stored by the Portfolio.
      using Inventory = typename Portfolio::PortfolioBookkeeper::Inventory;

      //! The type of updates published.
      using UpdateEntry = typename Portfolio::UpdateEntry;

      //! Constructs a PortfolioMonitor.
      /*!
        \param portfolio Initializes the Portfolio.
        \param marketDataClient Initializes the MarketDataClient.
        \param orderExecutionPublisher Publishes Order executions.
      */
      template<typename PortfolioForward, typename MarketDataClientForward>
      PortfolioMonitor(PortfolioForward&& portfolio,
        MarketDataClientForward&& marketDataClient,
        const OrderExecutionService::OrderExecutionPublisher&
        orderExecutionPublisher);

      //! Returns the object publishing updates to the monitored Portfolio.
      const Beam::SnapshotPublisher<UpdateEntry, Portfolio*>&
        GetPublisher() const;

    private:
      Beam::GetOptionalLocalPtr<PortfolioType> m_portfolio;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      OrderExecutionService::ExecutionReportPublisher
        m_executionReportPublisher;
      Beam::ValueSnapshotPublisher<UpdateEntry, Portfolio*> m_publisher;
      std::unordered_map<Security, BboQuote> m_bboQuotes;
      std::unordered_set<Security> m_securities;
      Beam::RoutineTaskQueue m_tasks;

      void PushUpdate(const Security& security);
      void OnBbo(const Security& security, const BboQuote& bbo);
      void OnExecutionReport(
        const OrderExecutionService::ExecutionReportEntry& executionReport);
  };

  template<typename PortfolioType, typename MarketDataClientType>
  template<typename PortfolioForward, typename MarketDataClientForward>
  PortfolioMonitor<PortfolioType, MarketDataClientType>::PortfolioMonitor(
      PortfolioForward&& portfolio, MarketDataClientForward&& marketDataClient,
      const OrderExecutionService::OrderExecutionPublisher&
      orderExecutionPublisher)
      : m_portfolio(std::forward<PortfolioForward>(portfolio)),
        m_marketDataClient(std::forward<MarketDataClientForward>(
          marketDataClient)),
        m_executionReportPublisher(orderExecutionPublisher),
        m_publisher(
          [] (auto snapshot, auto& monitor) {
            ForEachPortfolioEntry(*snapshot,
              [&] (auto& update) {
                monitor->Push(update);
              });
          }, Beam::SignalHandling::NullSlot(), &*m_portfolio) {
    m_executionReportPublisher.Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReportEntry>(
      std::bind(&PortfolioMonitor::OnExecutionReport, this,
      std::placeholders::_1)));
  }

  template<typename PortfolioType, typename MarketDataClientType>
  const Beam::SnapshotPublisher<typename PortfolioMonitor<PortfolioType,
      MarketDataClientType>::UpdateEntry, typename PortfolioMonitor<
      PortfolioType, MarketDataClientType>::Portfolio*>& PortfolioMonitor<
      PortfolioType, MarketDataClientType>::GetPublisher() const {
    return m_publisher;
  }

  template<typename PortfolioType, typename MarketDataClientType>
  void PortfolioMonitor<PortfolioType, MarketDataClientType>::PushUpdate(
      const Security& security) {
    auto securityEntryIterator = m_portfolio->GetSecurityEntries().find(
      security);
    if(securityEntryIterator == m_portfolio->GetSecurityEntries().end()) {
      return;
    }
    const auto& securityEntry = securityEntryIterator->second;
    if(!securityEntry.m_valuation.m_askValue.is_initialized() ||
        !securityEntry.m_valuation.m_bidValue.is_initialized()) {
      return;
    }
    UpdateEntry update;
    update.m_securityInventory = m_portfolio->GetBookkeeper().GetInventory(
      security, securityEntry.m_valuation.m_currency);
    if(update.m_securityInventory.m_transactionCount == 0) {
      return;
    }
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

  template<typename PortfolioType, typename MarketDataClientType>
  void PortfolioMonitor<PortfolioType, MarketDataClientType>::OnBbo(
      const Security& security, const BboQuote& bbo) {
    m_publisher.With(
      [&] {
        auto& lastBbo = m_bboQuotes[security];
        if(lastBbo.m_ask.m_price == bbo.m_ask.m_price &&
            lastBbo.m_bid.m_price == bbo.m_bid.m_price) {
          return;
        }
        lastBbo = bbo;
        if(bbo.m_ask.m_price == Money::ZERO) {
          if(bbo.m_bid.m_price == Money::ZERO) {
            return;
          }
          m_portfolio->UpdateBid(security, bbo.m_bid.m_price);
        } else if(bbo.m_bid.m_price == Money::ZERO) {
          m_portfolio->UpdateAsk(security, bbo.m_ask.m_price);
        } else {
          m_portfolio->Update(security, bbo.m_ask.m_price, bbo.m_bid.m_price);
        }
        PushUpdate(security);
      });
  }

  template<typename PortfolioType, typename MarketDataClientType>
  void PortfolioMonitor<PortfolioType, MarketDataClientType>::OnExecutionReport(
      const OrderExecutionService::ExecutionReportEntry& executionReport) {
    if(executionReport.m_executionReport.m_status == OrderStatus::PENDING_NEW) {
      auto& security = executionReport.m_order->GetInfo().m_fields.m_security;
      auto securityIterator = m_securities.find(security);
      if(securityIterator == m_securities.end()) {
        auto bboQuery = Beam::Queries::BuildCurrentQuery(security);
        m_marketDataClient->QueryBboQuotes(bboQuery,
          m_tasks.GetSlot<BboQuote>(std::bind(&PortfolioMonitor::OnBbo, this,
          security, std::placeholders::_1)));
        m_securities.insert(security);
      }
    }
    m_publisher.With(
      [&] {
        m_portfolio->Update(executionReport.m_order->GetInfo().m_fields,
          executionReport.m_executionReport);
        if(executionReport.m_executionReport.m_lastQuantity != 0) {
          PushUpdate(executionReport.m_order->GetInfo().m_fields.m_security);
        }
      });
  }
}

#endif
