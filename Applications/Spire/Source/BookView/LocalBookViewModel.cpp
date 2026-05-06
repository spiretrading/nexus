#include "Spire/BookView/LocalBookViewModel.hpp"
#include <ranges>
#include <sstream>
#include "Nexus/Definitions/FixTags.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

LocalBookViewModel::LocalBookViewModel()
  : m_model(std::make_shared<ReversedListModel<BookQuote>>(
      std::make_shared<ArrayListModel<BookQuote>>()),
      std::make_shared<ReversedListModel<BookQuote>>(
        std::make_shared<ArrayListModel<BookQuote>>()),
      std::make_shared<ArrayListModel<UserOrder>>(),
      std::make_shared<ArrayListModel<UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>(),
      std::make_shared<LocalBboQuoteModel>(),
      std::make_shared<LocalSessionCandlestickModel>()) {}

void LocalBookViewModel::update(const BboQuote& bbo) {
  m_model.get_bbo_quote()->set(bbo);
  update_pegged_orders();
}

void LocalBookViewModel::update(const BookQuote& quote) {
  auto direction = get_direction(quote.m_quote.m_side);
  auto quotes =
    pick(quote.m_quote.m_side, m_model.get_asks(), m_model.get_bids());
  auto lower_bound = [&] {
    for(auto i = quotes->begin(); i != quotes->end(); ++i) {
      if(direction * i->m_quote.m_price <= direction * quote.m_quote.m_price) {
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
      auto existing_index =
        std::ranges::distance(quotes->begin(), existing_iterator);
      auto insert_index =
        std::ranges::distance(quotes->begin(), insert_iterator);
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

void LocalBookViewModel::update(const TimeAndSale& time_and_sale) {
  auto candlestick = m_model.get_session_candlestick()->get();
  candlestick.update(time_and_sale.m_price, time_and_sale.m_size);
  m_model.get_session_candlestick()->set(candlestick);
}

void LocalBookViewModel::add(const OrderLogModel::OrderEntry& order) {
  add(order, order.m_order->get_info().m_fields.m_quantity,
    OrderStatus::PENDING_NEW);
}

void LocalBookViewModel::add(const OrderLogModel::OrderEntry& order,
    Quantity quantity, OrderStatus status) {
  auto& fields = order.m_order->get_info().m_fields;
  if(fields.m_type != OrderType::LIMIT && fields.m_type != OrderType::PEGGED) {
    return;
  }
  auto& orders = pick(fields.m_side, m_ask_orders, m_bid_orders);
  orders.push_back(order.m_order);
  auto& user_orders =
    *pick(fields.m_side, m_model.get_ask_orders(), m_model.get_bid_orders());
  auto display_price = fields.m_price;
  if(fields.m_type == OrderType::PEGGED) {
    submit_pegged(*order.m_order);
    display_price =
      m_pegged_entries[order.m_order->get_info().m_id].m_effective_price;
  }
  user_orders.push(
    UserOrder(fields.m_destination, display_price, quantity, status));
}

void LocalBookViewModel::remove(const OrderLogModel::OrderEntry& order) {
  auto& fields = order.m_order->get_info().m_fields;
  if(fields.m_type != OrderType::LIMIT && fields.m_type != OrderType::PEGGED) {
    return;
  }
  auto& orders = pick(fields.m_side, m_ask_orders, m_bid_orders);
  auto i = std::ranges::find(orders, order.m_order);
  if(i == orders.end()) {
    return;
  }
  auto index = static_cast<int>(std::ranges::distance(orders.begin(), i));
  auto& user_orders =
    *pick(fields.m_side, m_model.get_ask_orders(), m_model.get_bid_orders());
  m_pegged_entries.erase(order.m_order->get_info().m_id);
  orders.erase(i);
  user_orders.remove(index);
}

void LocalBookViewModel::update(const ExecutionReport& report) {
  auto find_order = [&] (auto& orders, auto& user_orders) -> optional<int> {
    for(auto i = 0; i != static_cast<int>(orders.size()); ++i) {
      if(orders[i]->get_info().m_id == report.m_id) {
        return i;
      }
    }
    return none;
  };
  auto update_order = [&] (int index, auto& user_orders) {
    auto user_order = user_orders.get(index);
    user_order.m_status = report.m_status;
    user_order.m_size -= report.m_last_quantity;
    user_orders.set(index, user_order);
    if(is_terminal(report.m_status)) {
      m_pegged_entries.erase(report.m_id);
    }
  };
  if(auto index = find_order(m_bid_orders, *m_model.get_bid_orders())) {
    update_order(*index, *m_model.get_bid_orders());
  } else if(auto index = find_order(m_ask_orders, *m_model.get_ask_orders())) {
    update_order(*index, *m_model.get_ask_orders());
  }
}

void LocalBookViewModel::clear_orders() {
  Spire::clear(*m_model.get_bid_orders());
  Spire::clear(*m_model.get_ask_orders());
  m_bid_orders.clear();
  m_ask_orders.clear();
  m_pegged_entries.clear();
}

void LocalBookViewModel::clear_book_quotes() {
  auto clear_side = [&] (auto& quotes) {
    auto cleared = std::vector(quotes.begin(), quotes.end());
    for(auto& quote : cleared) {
      if(!quote.m_mpid.empty()) {
        auto cleared_quote = quote;
        cleared_quote.m_quote.m_size = 0;
        update(cleared_quote);
      }
    }
  };
  clear_side(*m_model.get_asks());
  clear_side(*m_model.get_bids());
}

void LocalBookViewModel::transact(const std::function<void ()>& f) {
  m_model.get_asks()->transact([&] {
    m_model.get_bids()->transact([&] {
      f();
    });
  });
}

void LocalBookViewModel::submit_pegged(const Order& order) {
  auto& fields = order.get_info().m_fields;
  auto entry = PeggedOrderEntry();
  entry.m_exec_inst = PRIMARY_PEG;
  if(auto tag = find_field(fields, EXEC_INST_KEY)) {
    if(auto* value = boost::get<std::string>(&tag->get_value())) {
      auto stream = std::istringstream(*value);
      auto token = std::string();
      while(stream >> token) {
        if(token == PRIMARY_PEG || token == MARKET_PEG ||
            token == MID_PRICE_PEG) {
          entry.m_exec_inst = token;
          break;
        }
      }
    }
  }
  entry.m_peg_difference = Money::ZERO;
  if(auto tag = find_field(fields, PEG_DIFFERENCE_KEY)) {
    if(auto* money = boost::get<Money>(&tag->get_value())) {
      entry.m_peg_difference = *money;
    }
  }
  auto direction = get_direction(fields.m_side);
  auto& bbo = m_model.get_bbo_quote()->get();
  auto [same_price, opposite_price] = pick(fields.m_side,
    std::pair(bbo.m_ask.m_price, bbo.m_bid.m_price),
    std::pair(bbo.m_bid.m_price, bbo.m_ask.m_price));
  entry.m_effective_price = [&] {
    if(entry.m_exec_inst == MARKET_PEG) {
      return opposite_price;
    } else if(entry.m_exec_inst == MID_PRICE_PEG) {
      return (same_price + opposite_price) / 2;
    }
    return same_price;
  }();
  entry.m_effective_price -= direction * entry.m_peg_difference;
  auto limit_price = fields.m_price;
  if(limit_price != Money::ZERO &&
      direction * entry.m_effective_price > direction * limit_price) {
    entry.m_effective_price = limit_price;
  }
  m_pegged_entries[order.get_info().m_id] = entry;
}

void LocalBookViewModel::update_pegged_orders() {
  auto& bbo = m_model.get_bbo_quote()->get();
  auto update_side = [&] (auto& orders, auto& user_orders, Side side) {
    auto direction = get_direction(side);
    for(auto i = 0; i != static_cast<int>(orders.size()); ++i) {
      auto& order = orders[i];
      auto it = m_pegged_entries.find(order->get_info().m_id);
      if(it == m_pegged_entries.end()) {
        continue;
      }
      auto& entry = it->second;
      auto [same_price, opposite_price] = pick(side,
        std::pair(bbo.m_ask.m_price, bbo.m_bid.m_price),
        std::pair(bbo.m_bid.m_price, bbo.m_ask.m_price));
      auto candidate = [&] {
        if(entry.m_exec_inst == MARKET_PEG) {
          return opposite_price;
        } else if(entry.m_exec_inst == MID_PRICE_PEG) {
          return (same_price + opposite_price) / 2;
        }
        return same_price;
      }();
      candidate -= direction * entry.m_peg_difference;
      if(direction * candidate > direction * entry.m_effective_price) {
        entry.m_effective_price = candidate;
      }
      auto limit_price = order->get_info().m_fields.m_price;
      if(limit_price != Money::ZERO &&
          direction * entry.m_effective_price > direction * limit_price) {
        entry.m_effective_price = limit_price;
      }
      auto user_order = user_orders.get(i);
      if(user_order.m_price != entry.m_effective_price) {
        user_order.m_price = entry.m_effective_price;
        user_orders.set(i, user_order);
      }
    }
  };
  update_side(m_bid_orders, *m_model.get_bid_orders(), Side::BID);
  update_side(m_ask_orders, *m_model.get_ask_orders(), Side::ASK);
}

const std::shared_ptr<BookQuoteListModel>&
    LocalBookViewModel::get_bids() const {
  return m_model.get_bids();
}

const std::shared_ptr<BookQuoteListModel>&
    LocalBookViewModel::get_asks() const {
  return m_model.get_asks();
}

const std::shared_ptr<LocalBookViewModel::UserOrderListModel>&
    LocalBookViewModel::get_bid_orders() const {
  return m_model.get_bid_orders();
}

const std::shared_ptr<LocalBookViewModel::UserOrderListModel>&
    LocalBookViewModel::get_ask_orders() const {
  return m_model.get_ask_orders();
}

const std::shared_ptr<LocalBookViewModel::PreviewOrderModel>&
    LocalBookViewModel::get_preview_order() const {
  return m_model.get_preview_order();
}

const std::shared_ptr<BboQuoteModel>&
    LocalBookViewModel::get_bbo_quote() const {
  return m_model.get_bbo_quote();
}

const std::shared_ptr<SessionCandlestickModel>&
    LocalBookViewModel::get_session_candlestick() const {
  return m_model.get_session_candlestick();
}
