#ifndef NEXUS_PORTFOLIO_CONTROLLER_HPP
#define NEXUS_PORTFOLIO_CONTROLLER_HPP
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Queues/ValueSnapshotPublisher.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /**
   * Uses the BboQuote published by a MarketDataClient and the Orders published
   * by an OrderExecutionPublisher to update a Portfolio.
   * @param <P> The type of Portfolio to update.
   * @param <C> The type of MarketDataClient to use.
   */
  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  class PortfolioController {
    public:

      /** The type of Portfolio to update. */
      using Portfolio = Beam::GetTryDereferenceType<P>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a PortfolioController.
       * @param portfolio Initializes the Portfolio.
       * @param market_data_client Initializes the MarketDataClient.
       * @param orders The Orders to include in the Portfolio.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<C> MC>
      PortfolioController(PF&& portfolio, MC&& market_data_client,
        Beam::ScopedQueueReader<std::shared_ptr<const Order>> orders);

      /** Returns the object publishing updates to the Portfolio. */
      const Beam::SnapshotPublisher<PortfolioUpdateEntry, Portfolio*>&
        get_publisher() const;

    private:
      Beam::GetOptionalLocalPtr<P> m_portfolio;
      Beam::GetOptionalLocalPtr<C> m_market_data_client;
      ExecutionReportPublisher m_execution_report_publisher;
      Beam::ValueSnapshotPublisher<PortfolioUpdateEntry, Portfolio*>
        m_publisher;
      std::unordered_map<Security, BboQuote> m_bbo_quotes;
      std::unordered_set<Security> m_securities;
      Beam::RoutineTaskQueue m_tasks;

      PortfolioController(const PortfolioController&) = delete;
      PortfolioController& operator =(const PortfolioController&) = delete;
      void subscribe(const Security& security);
      void push_update(const Security& security);
      void on_bbo(const Security& security, const BboQuote& bbo);
      void on_execution_report(const ExecutionReportEntry& execution_report);
  };

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  PortfolioController(
    P&&, C&&, Beam::ScopedQueueReader<std::shared_ptr<const Order>>) ->
      PortfolioController<std::decay_t<P>, std::remove_reference_t<C>>;

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  template<Beam::Initializes<P> PF, Beam::Initializes<C> MC>
  PortfolioController<P, C>::PortfolioController(
      PF&& portfolio, MC&& market_data_client,
      Beam::ScopedQueueReader<std::shared_ptr<const Order>> orders)
    : m_portfolio(std::forward<PF>(portfolio)),
      m_market_data_client(
        std::forward<decltype(market_data_client)>(market_data_client)),
      m_execution_report_publisher(std::move(orders)),
      m_publisher([] (auto snapshot, auto& queue) {
        for_each(*snapshot, [&] (const auto& update) {
          queue.Push(update);
        });
      }, Beam::SignalHandling::NullSlot(), &*m_portfolio) {
    m_publisher.With([&] {
      for(auto& inventory :
          m_portfolio->get_bookkeeper().get_inventory_range()) {
        subscribe(inventory.m_position.m_security);
      }
      auto reports = boost::optional<std::vector<ExecutionReportEntry>>();
      m_execution_report_publisher.Monitor(
        m_tasks.GetSlot<ExecutionReportEntry>(
          std::bind_front(&PortfolioController::on_execution_report, this)),
        Beam::Store(reports));
      if(reports) {
        for(auto& report : *reports) {
          on_execution_report(report);
        }
      }
    });
  }

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  const Beam::SnapshotPublisher<PortfolioUpdateEntry,
    typename PortfolioController<P, C>::Portfolio*>&
      PortfolioController<P, C>::get_publisher() const {
    return m_publisher;
  }

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  void PortfolioController<P, C>::subscribe(const Security& security) {
    if(auto security_iterator = m_securities.find(security);
        security_iterator == m_securities.end()) {
      auto snapshot = std::make_shared<Beam::StateQueue<BboQuote>>();
      m_market_data_client->query(
        Beam::Queries::MakeLatestQuery(security), snapshot);
      try {
        on_bbo(security, snapshot->Pop());
      } catch(const std::exception&) {
      }
      m_market_data_client->query(Beam::Queries::MakeRealTimeQuery(security),
        m_tasks.GetSlot<BboQuote>(
          std::bind_front(&PortfolioController::on_bbo, this, security)));
      m_securities.insert(security);
    }
  }

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  void PortfolioController<P, C>::push_update(const Security& security) {
    auto security_entry_iterator =
      m_portfolio->get_security_entries().find(security);
    if(security_entry_iterator == m_portfolio->get_security_entries().end()) {
      return;
    }
    auto& security_entry = security_entry_iterator->second;
    auto& security_inventory = m_portfolio->get_bookkeeper().get_inventory(
      security, security_entry.m_valuation.m_currency);
    auto update = PortfolioUpdateEntry();
    update.m_security_inventory = security_inventory;
    update.m_unrealized_security = security_entry.m_unrealized;
    update.m_currency_inventory = m_portfolio->get_bookkeeper().get_total(
      security_entry.m_valuation.m_currency);
    auto unrealized_currency_iterator =
      m_portfolio->get_unrealized_profit_and_losses().find(
        security_entry.m_valuation.m_currency);
    if(unrealized_currency_iterator ==
        m_portfolio->get_unrealized_profit_and_losses().end()) {
      update.m_unrealized_currency = Money::ZERO;
    } else {
      update.m_unrealized_currency = unrealized_currency_iterator->second;
    }
    m_publisher.Push(update);
  }

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  void PortfolioController<P, C>::on_bbo(
      const Security& security, const BboQuote& bbo) {
    auto& last_bbo = m_bbo_quotes[security];
    if(last_bbo.m_ask.m_price == bbo.m_ask.m_price &&
        last_bbo.m_bid.m_price == bbo.m_bid.m_price) {
      return;
    }
    last_bbo = bbo;
    if(bbo.m_ask.m_price == Money::ZERO && bbo.m_bid.m_price == Money::ZERO) {
      return;
    }
    m_publisher.With([&] {
      auto has_update = [&] {
        if(bbo.m_ask.m_price == Money::ZERO) {
          return m_portfolio->update_bid(security, bbo.m_bid.m_price);
        } else if(bbo.m_bid.m_price == Money::ZERO) {
          return m_portfolio->update_ask(security, bbo.m_ask.m_price);
        }
        return m_portfolio->update(
          security, bbo.m_ask.m_price, bbo.m_bid.m_price);
      }();
      if(has_update) {
        push_update(security);
      }
    });
  }

  template<Beam::IsInstanceOrIndirect<Portfolio> P, IsMarketDataClient C>
  void PortfolioController<P, C>::on_execution_report(
      const ExecutionReportEntry& entry) {
    if(entry.m_report.m_status == OrderStatus::PENDING_NEW) {
      subscribe(entry.m_order->get_info().m_fields.m_security);
    }
    m_publisher.With([&] {
      if(m_portfolio->update(
          entry.m_order->get_info().m_fields, entry.m_report)) {
        push_update(entry.m_order->get_info().m_fields.m_security);
      }
    });
  }
}

#endif
