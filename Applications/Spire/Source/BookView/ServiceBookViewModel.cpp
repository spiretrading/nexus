#include "Spire/BookView/ServiceBookViewModel.hpp"
#include <algorithm>
#include <Beam/Queues/ScopedBaseQueue.hpp>

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto RETRY_INTERVAL = 3000;
}

struct ServiceBookViewModel::OrderEntry {
  std::shared_ptr<Order> m_order;
  ScopedBaseQueue<> m_subscription;

  explicit OrderEntry(std::shared_ptr<Order> order)
    : m_order(std::move(order)),
      m_subscription(nullptr) {}
};

ServiceBookViewModel::ServiceBookViewModel(Ticker ticker,
    MarketDataClient market_data_client,
    std::shared_ptr<const OrdersModel> orders)
    : m_ticker(std::move(ticker)),
      m_market_data_client(std::move(market_data_client)),
      m_orders(std::move(orders)),
      m_model(m_ticker) {
  if(!m_ticker) {
    return;
  }
  m_book_quote_retry_timer.setSingleShot(true);
  QObject::connect(&m_book_quote_retry_timer, &QTimer::timeout,
    std::bind_front(&ServiceBookViewModel::query_book_quotes, this));
  m_time_and_sale_retry_timer.setSingleShot(true);
  QObject::connect(&m_time_and_sale_retry_timer, &QTimer::timeout,
    std::bind_front(&ServiceBookViewModel::query_time_and_sales, this));
  auto bbo_query = make_current_query(m_ticker);
  bbo_query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
  m_market_data_client.query(bbo_query, m_event_handler.get_slot<BboQuote>(
    std::bind_front(&ServiceBookViewModel::on_bbo, this)));
  query_book_quotes();
  query_time_and_sales();
  m_model.transact([&] {
    for(auto i = 0; i != m_orders->get_size(); ++i) {
      add(i, m_orders->get(i));
    }
  });
  m_orders_connection = m_orders->connect_operation_signal(
    std::bind_front(&ServiceBookViewModel::on_order_operation, this));
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

const std::shared_ptr<SessionTechnicalsModel>&
    ServiceBookViewModel::get_session_technicals() const {
  return m_model.get_session_technicals();
}

void ServiceBookViewModel::add(int index, std::shared_ptr<Order> order) {
  if(order->get_info().m_fields.m_ticker != m_ticker) {
    m_order_entries.insert(m_order_entries.begin() + index, nullptr);
    return;
  }
  auto entry = std::make_shared<OrderEntry>(std::move(order));
  m_order_entries.insert(m_order_entries.begin() + index, entry);
  auto subscription = std::make_shared<ScopedQueueWriter<ExecutionReport>>(
    m_order_events.get_slot<ExecutionReport>(
      [=, this, weak_entry = std::weak_ptr(entry)] (const auto& report) {
        if(weak_entry.lock()) {
          m_model.update(report);
        }
      }));
  entry->m_subscription = ScopedBaseQueue(subscription);
  auto snapshot = optional<std::vector<ExecutionReport>>();
  entry->m_order->get_publisher().monitor(
    std::move(subscription), out(snapshot));
  auto quantity = entry->m_order->get_info().m_fields.m_quantity;
  auto status = OrderStatus::PENDING_NEW;
  if(snapshot) {
    for(auto& report : *snapshot) {
      quantity -= report.m_last_quantity;
      status = report.m_status;
    }
  }
  if(!is_terminal(status)) {
    m_model.add(entry->m_order, quantity, status);
  }
}

void ServiceBookViewModel::remove(int index) {
  auto entry = std::move(m_order_entries[index]);
  m_order_entries.erase(m_order_entries.begin() + index);
  if(entry) {
    m_model.remove(entry->m_order);
  }
}

void ServiceBookViewModel::query_book_quotes() {
  auto slot = m_event_handler.get_slot<BookQuote>(
    std::bind_front(&ServiceBookViewModel::buffer_book_quote, this),
    std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this));
  spawn([client = m_market_data_client, ticker = m_ticker,
      slot = std::move(slot)] () mutable {
    auto id = query_real_time_with_snapshot(
      client, ticker, std::move(slot), InterruptionPolicy::BREAK_QUERY);
    Beam::wait(id);
  });
}

void ServiceBookViewModel::query_time_and_sales() {
  auto technicals = m_event_handler.get_slot<SessionTechnicals>(
    std::bind_front(&ServiceBookViewModel::on_session_technicals, this));
  auto trades = m_event_handler.get_slot<TimeAndSale>(
    std::bind_front(&ServiceBookViewModel::on_time_and_sales, this),
    std::bind_front(
      &ServiceBookViewModel::on_time_and_sales_interruption, this));
  spawn([client = m_market_data_client, ticker = m_ticker,
      technicals = std::move(technicals),
      trades = std::move(trades)] () mutable {
    auto snapshot = SequencedSessionTechnicals();
    try {
      snapshot = client.load_session_technicals(ticker);
    } catch(const std::exception&) {
      technicals.close(std::current_exception());
      trades.close(std::current_exception());
      return;
    }
    technicals.push(*snapshot);
    auto query = TickerQuery();
    query.set_index(std::move(ticker));
    query.set_range(
      increment(snapshot.get_sequence()), Beam::Sequence::LAST);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    client.query(query, std::move(trades));
  });
}

void ServiceBookViewModel::buffer_book_quote(const BookQuote& quote) {
  m_buffered_book_quotes.push_back(quote);
  if(m_buffered_book_quotes.size() == 1) {
    m_event_handler.push(
      std::bind_front(&ServiceBookViewModel::on_end_book_quote_buffer, this));
  }
}

void ServiceBookViewModel::on_order_operation(
    const OrdersModel::Operation& operation) {
  visit(operation,
    [&] (const OrdersModel::AddOperation& operation) {
      m_model.transact([&] {
        add(operation.m_index, m_orders->get(operation.m_index));
      });
    },
    [&] (const OrdersModel::RemoveOperation& operation) {
      m_model.transact([&] {
        remove(operation.m_index);
      });
    },
    [&] (const OrdersModel::MoveOperation& operation) {
      auto first = m_order_entries.begin() + operation.m_source;
      auto last = m_order_entries.begin() + operation.m_destination;
      if(first < last) {
        std::ranges::rotate(first, first + 1, last + 1);
      } else {
        std::ranges::rotate(last, first, first + 1);
      }
    },
    [&] (const OrdersModel::UpdateOperation& operation) {
      if(operation.get_previous() != operation.get_value()) {
        m_model.transact([&] {
          remove(operation.m_index);
          add(operation.m_index, operation.get_value());
        });
      }
    });
}

void ServiceBookViewModel::on_bbo(const BboQuote& bbo) {
  m_model.update(bbo);
}

void ServiceBookViewModel::on_end_book_quote_buffer() {
  auto quotes = std::vector<BookQuote>();
  quotes.swap(m_buffered_book_quotes);
  m_model.transact([&] {
    for(auto& quote : quotes) {
      m_model.update(quote);
    }
  });
}

void ServiceBookViewModel::on_book_quote_interruption(
    const std::exception_ptr&) {
  m_buffered_book_quotes.clear();
  m_model.clear_book_quotes();
  m_book_quote_retry_timer.start(RETRY_INTERVAL);
}

void ServiceBookViewModel::on_session_technicals(
    const SessionTechnicals& technicals) {
  m_model.get_session_technicals()->set(technicals);
}

void ServiceBookViewModel::on_time_and_sales(const TimeAndSale& time_and_sale) {
  m_model.update(time_and_sale);
}

void ServiceBookViewModel::on_time_and_sales_interruption(
    const std::exception_ptr&) {
  m_time_and_sale_retry_timer.start(RETRY_INTERVAL);
}
