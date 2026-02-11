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
  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  class PortfolioController {
    public:

      /** The type of Portfolio to update. */
      using Portfolio = Beam::dereference_t<P>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::dereference_t<C>;

      /**
       * Constructs a PortfolioController.
       * @param portfolio Initializes the Portfolio.
       * @param market_data_client Initializes the MarketDataClient.
       * @param orders The Orders to include in the Portfolio.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<C> MC>
      PortfolioController(PF&& portfolio, MC&& market_data_client,
        Beam::ScopedQueueReader<std::shared_ptr<Order>> orders);

      /** Returns the object publishing updates to the Portfolio. */
      const Beam::SnapshotPublisher<PortfolioUpdateEntry, Portfolio*>&
        get_publisher() const;

    private:
      Beam::local_ptr_t<P> m_portfolio;
      Beam::local_ptr_t<C> m_market_data_client;
      ExecutionReportPublisher m_execution_report_publisher;
      Beam::ValueSnapshotPublisher<PortfolioUpdateEntry, Portfolio*>
        m_publisher;
      std::unordered_map<Ticker, BboQuote> m_bbo_quotes;
      std::unordered_set<Ticker> m_tickers;
      Beam::RoutineTaskQueue m_tasks;

      PortfolioController(const PortfolioController&) = delete;
      PortfolioController& operator =(const PortfolioController&) = delete;
      void subscribe(const Ticker& ticker);
      void push_update(const Ticker& ticker);
      void on_bbo(const Ticker& ticker, const BboQuote& bbo);
      void on_execution_report(const ExecutionReportEntry& execution_report);
  };

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  PortfolioController(
    P&&, C&&, Beam::ScopedQueueReader<std::shared_ptr<Order>>) ->
      PortfolioController<std::decay_t<P>, std::remove_reference_t<C>>;

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  template<Beam::Initializes<P> PF, Beam::Initializes<C> MC>
  PortfolioController<P, C>::PortfolioController(
      PF&& portfolio, MC&& market_data_client,
      Beam::ScopedQueueReader<std::shared_ptr<Order>> orders)
    : m_portfolio(std::forward<PF>(portfolio)),
      m_market_data_client(
        std::forward<decltype(market_data_client)>(market_data_client)),
      m_execution_report_publisher(std::move(orders)),
      m_publisher([] (auto snapshot, auto& queue) {
        for_each(*snapshot, [&] (const auto& update) {
          queue.push(update);
        });
      }, Beam::NullSlot(), &*m_portfolio) {
    m_publisher.with([&] {
      for(auto& inventory :
          m_portfolio->get_bookkeeper().get_inventory_range()) {
        subscribe(inventory.m_position.m_ticker);
      }
      auto reports = boost::optional<std::vector<ExecutionReportEntry>>();
      m_execution_report_publisher.monitor(
        m_tasks.get_slot<ExecutionReportEntry>(
          std::bind_front(&PortfolioController::on_execution_report, this)),
        Beam::out(reports));
      if(reports) {
        for(auto& report : *reports) {
          on_execution_report(report);
        }
      }
    });
  }

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  const Beam::SnapshotPublisher<PortfolioUpdateEntry,
    typename PortfolioController<P, C>::Portfolio*>&
      PortfolioController<P, C>::get_publisher() const {
    return m_publisher;
  }

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  void PortfolioController<P, C>::subscribe(const Ticker& ticker) {
    if(auto ticker_iterator = m_tickers.find(ticker);
        ticker_iterator == m_tickers.end()) {
      auto snapshot = std::make_shared<Beam::StateQueue<BboQuote>>();
      m_market_data_client->query(Beam::make_latest_query(ticker), snapshot);
      try {
        on_bbo(ticker, snapshot->pop());
      } catch(const std::exception&) {
      }
      m_market_data_client->query(Beam::make_real_time_query(ticker),
        m_tasks.get_slot<BboQuote>(
          std::bind_front(&PortfolioController::on_bbo, this, ticker)));
      m_tickers.insert(ticker);
    }
  }

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  void PortfolioController<P, C>::push_update(const Ticker& ticker) {
    auto ticker_entry_iterator = m_portfolio->get_entries().find(ticker);
    if(ticker_entry_iterator == m_portfolio->get_entries().end()) {
      return;
    }
    auto& ticker_entry = ticker_entry_iterator->second;
    auto& ticker_inventory =
      m_portfolio->get_bookkeeper().get_inventory(ticker);
    auto update = PortfolioUpdateEntry();
    update.m_inventory = ticker_inventory;
    update.m_unrealized = ticker_entry.m_unrealized;
    update.m_currency_inventory = m_portfolio->get_bookkeeper().get_total(
      ticker_entry.m_valuation.m_currency);
    auto unrealized_currency_iterator =
      m_portfolio->get_unrealized_profit_and_losses().find(
        ticker_entry.m_valuation.m_currency);
    if(unrealized_currency_iterator ==
        m_portfolio->get_unrealized_profit_and_losses().end()) {
      update.m_unrealized_currency = Money::ZERO;
    } else {
      update.m_unrealized_currency = unrealized_currency_iterator->second;
    }
    m_publisher.push(update);
  }

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  void PortfolioController<P, C>::on_bbo(
      const Ticker& ticker, const BboQuote& bbo) {
    auto& last_bbo = m_bbo_quotes[ticker];
    if(last_bbo.m_ask.m_price == bbo.m_ask.m_price &&
        last_bbo.m_bid.m_price == bbo.m_bid.m_price) {
      return;
    }
    last_bbo = bbo;
    if(bbo.m_ask.m_price == Money::ZERO && bbo.m_bid.m_price == Money::ZERO) {
      return;
    }
    m_publisher.with([&] {
      auto has_update = [&] {
        if(bbo.m_ask.m_price == Money::ZERO) {
          return m_portfolio->update_bid(ticker, bbo.m_bid.m_price);
        } else if(bbo.m_bid.m_price == Money::ZERO) {
          return m_portfolio->update_ask(ticker, bbo.m_ask.m_price);
        }
        return m_portfolio->update(
          ticker, bbo.m_ask.m_price, bbo.m_bid.m_price);
      }();
      if(has_update) {
        push_update(ticker);
      }
    });
  }

  template<typename P, typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  void PortfolioController<P, C>::on_execution_report(
      const ExecutionReportEntry& entry) {
    if(entry.m_report.m_status == OrderStatus::PENDING_NEW) {
      subscribe(entry.m_order->get_info().m_fields.m_ticker);
    }
    m_publisher.with([&] {
      if(m_portfolio->update(
          entry.m_order->get_info().m_fields, entry.m_report)) {
        push_update(entry.m_order->get_info().m_fields.m_ticker);
      }
    });
  }
}

#endif
