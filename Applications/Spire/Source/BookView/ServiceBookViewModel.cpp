#include "Spire/BookView/ServiceBookViewModel.hpp"
#include <ranges>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/BookView/AggregateBookViewModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  bool is_order_displayed(const Order& order, const Security& security) {
    return order.get_info().m_fields.m_security == security &&
      order.get_info().m_fields.m_type == OrderType::LIMIT;
  }

  optional<std::tuple<std::vector<std::shared_ptr<Order>>&, int,
      BookViewModel::UserOrderListModel&>> pick(std::shared_ptr<Order> order,
        std::vector<std::shared_ptr<Order>>& ask_orders,
        std::vector<std::shared_ptr<Order>> bid_orders, BookViewModel& model) {
    auto& fields = order->get_info().m_fields;
    auto& orders = pick(fields.m_side, ask_orders, bid_orders);
    auto i = std::find_if(orders.begin(), orders.end(),
      [&] (const auto& element) {
        return order == element;
      });
    if(i == orders.end()) {
      return none;
    }
    auto index = std::distance(orders.begin(), i);
    auto& user_orders = *pick(
      fields.m_side, model.get_ask_orders(), model.get_bid_orders());
    return std::tuple<std::vector<std::shared_ptr<Order>>&, int,
      BookViewModel::UserOrderListModel&>(orders, index, user_orders);
  }
}

ServiceBookViewModel::ServiceBookViewModel(
    Security security, BlotterSettings& blotter, MarketDataClient client)
    : m_security(std::move(security)),
      m_blotter(&blotter),
      m_client(std::move(client)),
      m_model(make_local_aggregate_book_view_model()),
      m_bid_quotes(
        std::make_shared<ReversedListModel<BookQuote>>(m_model->get_bids())),
      m_ask_quotes(
        std::make_shared<ReversedListModel<BookQuote>>(m_model->get_asks())) {
  if(!m_security) {
    return;
  }
  auto bbo_query = make_current_query(m_security);
  bbo_query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
  m_client.query(bbo_query, m_event_handler.get_slot<BboQuote>(
    std::bind_front(&ServiceBookViewModel::on_bbo, this)));
  query_real_time_with_snapshot(m_client, m_security,
    m_event_handler.get_slot<BookQuote>(
      std::bind_front(&ServiceBookViewModel::buffer_book_quote, this),
      std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
  auto time_and_sale_query = make_real_time_query(m_security);
  time_and_sale_query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(time_and_sale_query, m_event_handler.get_slot<TimeAndSale>(
    std::bind_front(&ServiceBookViewModel::on_time_and_sales, this)));
  m_load_promise = std::make_shared<QtPromise<void>>(QtPromise([=] {
    return m_client.load_technicals(m_security);
  }, LaunchPolicy::ASYNC).then([model = m_model] (const auto& technicals) {
    model->get_technicals()->set(technicals);
  }));
  on_active_blotter(m_blotter->GetActiveBlotter());
  m_active_blotter_connection = m_blotter->ConnectActiveBlotterChangedSignal(
    std::bind_front(&ServiceBookViewModel::on_active_blotter, this));
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_bids() const {
  return m_bid_quotes;
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_asks() const {
  return m_ask_quotes;
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_bid_orders() const {
  return m_model->get_bid_orders();
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_ask_orders() const {
  return m_model->get_ask_orders();
}

const std::shared_ptr<BookViewModel::PreviewOrderModel>&
    ServiceBookViewModel::get_preview_order() const {
  return m_model->get_preview_order();
}

const std::shared_ptr<BboQuoteModel>&
    ServiceBookViewModel::get_bbo_quote() const {
  return m_model->get_bbo_quote();
}

const std::shared_ptr<SecurityTechnicalsModel>&
    ServiceBookViewModel::get_technicals() const {
  return m_model->get_technicals();
}

void ServiceBookViewModel::on_bbo(const BboQuote& bbo) {
  m_model->get_bbo_quote()->set(bbo);
}

void ServiceBookViewModel::buffer_book_quote(const Nexus::BookQuote& quote) {
  m_buffered_book_quotes.push_back(quote);
  if(m_buffered_book_quotes.size() == 1) {
    m_event_handler.push(
      std::bind_front(&ServiceBookViewModel::on_end_book_quote_buffer, this));
  }
}

void ServiceBookViewModel::on_end_book_quote_buffer() {
  get_asks()->transact([&] {
    get_bids()->transact([&] {
      for(auto& quote : m_buffered_book_quotes) {
        on_book_quote(quote);
      }
    });
  });
  m_buffered_book_quotes.clear();
}

void ServiceBookViewModel::on_book_quote(const BookQuote& quote) {
  auto direction = get_direction(quote.m_quote.m_side);
  auto quotes = pick(quote.m_quote.m_side, get_asks(), get_bids());
  auto lower_bound = [&] {
    for(auto i = quotes->begin(); i != quotes->end(); ++i) {
      auto& book_quote = i->m_quote;
      if(direction * book_quote.m_price <= direction * quote.m_quote.m_price) {
        return i;
      }
    }
    return quotes->end();
  }();
  auto existing_iterator = lower_bound;
  while(existing_iterator != quotes->end() &&
      existing_iterator->m_quote.m_price == quote.m_quote.m_price &&
      existing_iterator->m_mpid != quote.m_mpid) {
    ++existing_iterator;
  }
  if(existing_iterator == quotes->end() ||
      existing_iterator->m_quote.m_price != quote.m_quote.m_price) {
    if(quote.m_quote.m_size != 0) {
      auto insert_iterator = lower_bound;
      while(insert_iterator != quotes->end() &&
          insert_iterator->m_quote.m_price == quote.m_quote.m_price &&
          std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
          std::tie(insert_iterator->m_quote.m_size,
            insert_iterator->m_timestamp, insert_iterator->m_mpid)) {
        ++insert_iterator;
      }
      quotes->insert(quote, insert_iterator);
    }
    return;
  }
  if(quote.m_quote.m_size == 0) {
    quotes->remove(existing_iterator);
  } else {
    auto insert_iterator = lower_bound;
    while(insert_iterator != quotes->end() &&
        insert_iterator->m_quote.m_price == quote.m_quote.m_price &&
        std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
        std::tie(insert_iterator->m_quote.m_size,
          insert_iterator->m_timestamp, insert_iterator->m_mpid)) {
      ++insert_iterator;
    }
    if(insert_iterator == existing_iterator) {
      *insert_iterator = quote;
    } else {
      auto existing_index = std::distance(quotes->begin(), existing_iterator);
      auto insert_index = std::distance(quotes->begin(), insert_iterator);
      if(insert_index > existing_index) {
        --insert_index;
        if(insert_index == existing_index) {
          *existing_iterator = quote;
          return;
        }
      }
      quotes->remove(existing_index);
      quotes->insert(quote, insert_index);
    }
  }
}

void ServiceBookViewModel::clear(const BookQuoteListModel& quotes) {
  auto cleared_quotes = std::vector<BookQuote>(quotes.begin(), quotes.end());
  for(auto& quote : cleared_quotes) {
    if(!quote.m_mpid.empty()) {
      auto cleared_quote = quote;
      quote.m_quote.m_size = 0;
      on_book_quote(cleared_quote);
    }
  }
}

void ServiceBookViewModel::on_book_quote_interruption(
    const std::exception_ptr&) {
  clear(*get_asks());
  clear(*get_bids());
  query_real_time_with_snapshot(
    m_client, m_security, m_event_handler.get_slot<BookQuote>(
      std::bind_front(&ServiceBookViewModel::on_book_quote, this),
      std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
}

void ServiceBookViewModel::on_time_and_sales(const TimeAndSale& time_and_sale) {
  auto technicals = m_model->get_technicals()->get();
  technicals.m_volume += time_and_sale.m_size;
  if(technicals.m_open == Money::ZERO) {
    technicals.m_open = time_and_sale.m_price;
  }
  if(time_and_sale.m_price > technicals.m_high) {
    technicals.m_high = time_and_sale.m_price;
  }
  if(time_and_sale.m_price < technicals.m_low ||
      technicals.m_low == Money::ZERO) {
    technicals.m_low = time_and_sale.m_price;
  }
  m_model->get_technicals()->set(technicals);
}

void ServiceBookViewModel::on_execution_report(
    const std::shared_ptr<Order>& order, const ExecutionReport& report) {
  auto entry = pick(order, m_ask_orders, m_bid_orders, *m_model);
  if(!entry) {
    return;
  }
  auto [orders, index, user_orders] = *entry;
  auto user_order = user_orders.get(index);
  user_order.m_status = report.m_status;
  user_order.m_size -= report.m_last_quantity;
  user_orders.set(index, user_order);
}

void ServiceBookViewModel::on_order_added(
    const OrderLogModel::OrderEntry& order) {
  if(!is_order_displayed(*order.m_order, m_security)) {
    return;
  }
  auto& fields = order.m_order->get_info().m_fields;
  auto& orders = pick(fields.m_side, m_ask_orders, m_bid_orders);
  orders.push_back(order.m_order);
  auto& user_orders =
    *pick(fields.m_side, m_model->get_ask_orders(), m_model->get_bid_orders());
  auto execution_reports = optional<std::vector<ExecutionReport>>();
  order.m_order->get_publisher().monitor(
    m_order_event_handler->get_slot<ExecutionReport>(std::bind_front(
      &ServiceBookViewModel::on_execution_report, this, order.m_order)),
      out(execution_reports));
  auto remaining_quantity = fields.m_quantity;
  auto status = OrderStatus::PENDING_NEW;
  if(execution_reports) {
    for(auto& report : *execution_reports) {
      remaining_quantity -= report.m_last_quantity;
      status = report.m_status;
    }
  }
  user_orders.push(UserOrder(
    fields.m_destination, fields.m_price, remaining_quantity, status));
}

void ServiceBookViewModel::on_order_removed(
    const OrderLogModel::OrderEntry& order) {
  if(!is_order_displayed(*order.m_order, m_security)) {
    return;
  }
  auto entry = pick(order.m_order, m_ask_orders, m_bid_orders, *m_model);
  if(!entry) {
    return;
  }
  auto [orders, index, user_orders] = *entry;
  orders.erase(orders.begin() + index);
  user_orders.remove(index);
}

void ServiceBookViewModel::on_active_blotter(BlotterModel& blotter) {
  m_order_event_handler.reset();
  m_order_event_handler.emplace();
  auto& orders = blotter.GetOrderLogModel();
  auto reinitialize = [&] (UserOrderListModel& user_orders, Side side) {
    user_orders.transact([&] {
      Spire::clear(user_orders);
      for(auto i = 0; i != orders.rowCount(orders.index(0, 0)); ++i) {
        auto& entry = orders.GetEntry(orders.index(i, 0));
        auto& fields = entry.m_order->get_info().m_fields;
        if(fields.m_side == side) {
          on_order_added(entry);
        }
      }
    });
  };
  reinitialize(*m_model->get_ask_orders(), Side::ASK);
  reinitialize(*m_model->get_bid_orders(), Side::BID);
  m_order_added_connection = orders.ConnectOrderAddedSignal(
    std::bind_front(&ServiceBookViewModel::on_order_added, this));
  m_order_removed_connection = orders.ConnectOrderRemovedSignal(
    std::bind_front(&ServiceBookViewModel::on_order_removed, this));
}
