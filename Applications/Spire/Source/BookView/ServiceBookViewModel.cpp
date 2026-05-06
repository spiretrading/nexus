#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  bool is_order_displayed(const Order& order, const Ticker& ticker) {
    return order.get_info().m_fields.m_ticker == ticker &&
      (order.get_info().m_fields.m_type == OrderType::LIMIT ||
        order.get_info().m_fields.m_type == OrderType::PEGGED);
  }
}

ServiceBookViewModel::ServiceBookViewModel(Ticker ticker,
    BlotterSettings& blotter, MarketDataClient market_data_client,
    TimeClient time_client)
    : m_ticker(std::move(ticker)),
      m_blotter(&blotter),
      m_market_data_client(std::move(market_data_client)),
      m_time_client(std::move(time_client)) {
  if(!m_ticker) {
    return;
  }
  auto bbo_query = make_current_query(m_ticker);
  bbo_query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
  m_market_data_client.query(bbo_query, m_event_handler.get_slot<BboQuote>(
    std::bind_front(&ServiceBookViewModel::on_bbo, this)));
  query_real_time_with_snapshot(
    m_market_data_client, m_ticker, m_event_handler.get_slot<BookQuote>(
      std::bind_front(&ServiceBookViewModel::buffer_book_quote, this),
      std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
  auto time_and_sale_query = make_real_time_query(m_ticker);
  time_and_sale_query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_market_data_client.query(
    time_and_sale_query, m_event_handler.get_slot<TimeAndSale>(
      std::bind_front(&ServiceBookViewModel::on_time_and_sales, this)));
  m_load_promise = std::make_shared<QtPromise<void>>(QtPromise([this] {
    return m_market_data_client.load_session_candlestick(m_ticker);
  }, LaunchPolicy::ASYNC).then([this] (const auto& candlestick) {
    m_model.get_session_candlestick()->set(candlestick);
  }));
  on_active_blotter(m_blotter->GetActiveBlotter());
  m_active_blotter_connection = m_blotter->ConnectActiveBlotterChangedSignal(
    std::bind_front(&ServiceBookViewModel::on_active_blotter, this));
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_bids() const {
  return m_model.get_bids();
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_asks() const {
  return m_model.get_asks();
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_bid_orders() const {
  return m_model.get_bid_orders();
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_ask_orders() const {
  return m_model.get_ask_orders();
}

const std::shared_ptr<BookViewModel::PreviewOrderModel>&
    ServiceBookViewModel::get_preview_order() const {
  return m_model.get_preview_order();
}

const std::shared_ptr<BboQuoteModel>&
    ServiceBookViewModel::get_bbo_quote() const {
  return m_model.get_bbo_quote();
}

const std::shared_ptr<SessionCandlestickModel>&
    ServiceBookViewModel::get_session_candlestick() const {
  return m_model.get_session_candlestick();
}

void ServiceBookViewModel::initialize_order(
    const OrderLogModel::OrderEntry& order) {
  if(!is_order_displayed(*order.m_order, m_ticker)) {
    return;
  }
  auto execution_reports = optional<std::vector<ExecutionReport>>();
  order.m_order->get_publisher().monitor(
    m_order_event_handler->get_slot<ExecutionReport>(
      std::bind_front(&ServiceBookViewModel::on_execution_report, this)),
    out(execution_reports));
  if(execution_reports && !execution_reports->empty() &&
      is_terminal(execution_reports->back().m_status)) {
    return;
  }
  auto filled_quantity = Quantity(0);
  auto status = OrderStatus::PENDING_NEW;
  if(execution_reports) {
    for(auto& report : *execution_reports) {
      filled_quantity += report.m_last_quantity;
      if(report.m_status == OrderStatus::NEW) {
        status = OrderStatus::NEW;
      }
    }
  }
  auto& fields = order.m_order->get_info().m_fields;
  m_model.add(order, fields.m_quantity - filled_quantity, status);
}

void ServiceBookViewModel::buffer_book_quote(const BookQuote& quote) {
  m_buffered_book_quotes.push_back(quote);
  if(m_buffered_book_quotes.size() == 1) {
    m_event_handler.push(
      std::bind_front(&ServiceBookViewModel::on_end_book_quote_buffer, this));
  }
}

void ServiceBookViewModel::on_bbo(const BboQuote& bbo) {
  m_model.update(bbo);
}

void ServiceBookViewModel::on_end_book_quote_buffer() {
  m_model.transact([&] {
    for(auto& quote : m_buffered_book_quotes) {
      m_model.update(quote);
    }
  });
  m_buffered_book_quotes.clear();
}

void ServiceBookViewModel::on_book_quote_interruption(
    const std::exception_ptr&) {
  m_model.clear_book_quotes();
  query_real_time_with_snapshot(
    m_market_data_client, m_ticker, m_event_handler.get_slot<BookQuote>(
      std::bind_front(&ServiceBookViewModel::buffer_book_quote, this),
      std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
}

void ServiceBookViewModel::on_time_and_sales(const TimeAndSale& time_and_sale) {
  m_model.update(time_and_sale);
}

void ServiceBookViewModel::on_execution_report(const ExecutionReport& report) {
  m_model.update(report);
}

void ServiceBookViewModel::on_order_added(
    const OrderLogModel::OrderEntry& order) {
  if(order.m_order->get_info().m_timestamp < m_snapshot_cutoff) {
    initialize_order(order);
  } else {
    if(!is_order_displayed(*order.m_order, m_ticker)) {
      return;
    }
    m_model.add(order);
    auto execution_reports = optional<std::vector<ExecutionReport>>();
    order.m_order->get_publisher().monitor(
      m_order_event_handler->get_slot<ExecutionReport>(
        std::bind_front(&ServiceBookViewModel::on_execution_report, this)),
      out(execution_reports));
    if(execution_reports) {
      for(auto& report : *execution_reports) {
        m_model.update(report);
      }
    }
  }
}

void ServiceBookViewModel::on_order_removed(
    const OrderLogModel::OrderEntry& order) {
  if(!is_order_displayed(*order.m_order, m_ticker)) {
    return;
  }
  m_model.remove(order);
}

void ServiceBookViewModel::on_active_blotter(BlotterModel& blotter) {
  m_snapshot_cutoff = m_time_client.get_time();
  m_order_event_handler.reset();
  m_order_event_handler.emplace();
  auto& orders = blotter.GetOrderLogModel();
  m_model.transact([&] {
    m_model.clear_orders();
    for(auto i = 0; i != orders.rowCount(orders.index(0, 0)); ++i) {
      auto& entry = orders.GetEntry(orders.index(i, 0));
      initialize_order(entry);
    }
  });
  m_order_added_connection = orders.ConnectOrderAddedSignal(
    std::bind_front(&ServiceBookViewModel::on_order_added, this));
  m_order_removed_connection = orders.ConnectOrderRemovedSignal(
    std::bind_front(&ServiceBookViewModel::on_order_removed, this));
}
