#include "Spire/BookView/LocalBookViewModel.hpp"
#include <algorithm>
#include <ranges>
#include <sstream>
#include <boost/iterator/counting_iterator.hpp>
#include "Nexus/Definitions/FixTags.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/TechnicalAnalysis/SessionTechnicals.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  Money compute_peg_price(
      const std::string& exec_inst, const BboQuote& bbo, Side side) {
    auto [same_price, opposite_price] = pick(side,
      std::pair(bbo.m_ask.m_price, bbo.m_bid.m_price),
      std::pair(bbo.m_bid.m_price, bbo.m_ask.m_price));
    if(exec_inst == MARKET_PEG) {
      return opposite_price;
    } else if(exec_inst == MID_PRICE_PEG) {
      return (same_price + opposite_price) / 2;
    }
    return same_price;
  }

  Money clamp_to_limit(Money price, Money limit, Side side) {
    auto direction = get_direction(side);
    if(limit != Money::ZERO && direction * price > direction * limit) {
      return limit;
    }
    return price;
  }

  int find_partition_point(const BookQuoteListModel& quotes, auto is_before) {
    auto size = quotes.get_size();
    if(size == 0 || !is_before(quotes.get(0))) {
      return 0;
    }
    auto lower = 0;
    auto upper = 1;
    while(upper < size && is_before(quotes.get(upper))) {
      lower = upper;
      upper *= 2;
    }
    upper = std::min(upper, size);
    return *std::partition_point(make_counting_iterator(lower + 1),
      make_counting_iterator(upper), [&] (auto index) {
        return is_before(quotes.get(index));
      });
  }
}

LocalBookViewModel::LocalBookViewModel(Ticker ticker)
  : m_model(std::make_shared<ReversedListModel<BookQuote>>(
      std::make_shared<ArrayListModel<BookQuote>>()),
      std::make_shared<ReversedListModel<BookQuote>>(
        std::make_shared<ArrayListModel<BookQuote>>()),
      std::make_shared<ArrayListModel<UserOrder>>(),
      std::make_shared<ArrayListModel<UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>(),
      std::make_shared<LocalBboQuoteModel>(),
      std::make_shared<LocalSessionTechnicalsModel>()) {
  if(ticker) {
    m_market_center = VENUES.from(ticker.get_venue()).m_market_center;
    if(m_market_center.empty()) {
      m_market_center = ticker.get_venue().get_code().get_data();
    }
  }
}

void LocalBookViewModel::update(const BboQuote& bbo) {
  m_model.get_bbo_quote()->set(bbo);
  update_pegged_orders();
}

void LocalBookViewModel::update(const BookQuote& quote) {
  auto direction = get_direction(quote.m_quote.m_side);
  auto quotes =
    pick(quote.m_quote.m_side, m_model.get_asks(), m_model.get_bids());
  auto lower_bound = std::next(
    quotes->begin(), find_partition_point(*quotes, [&] (const auto& entry) {
      return direction * entry.m_quote.m_price >
        direction * quote.m_quote.m_price;
    }));
  auto find_insert_position = [&] {
    auto i = lower_bound;
    while(i != quotes->end() && i->m_quote.m_price == quote.m_quote.m_price &&
        std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
          std::tie(i->m_quote.m_size, i->m_timestamp, i->m_mpid)) {
      ++i;
    }
    return i;
  };
  auto existing_iterator = lower_bound;
  while(existing_iterator != quotes->end() &&
      existing_iterator->m_quote.m_price == quote.m_quote.m_price &&
      existing_iterator->m_mpid != quote.m_mpid) {
    ++existing_iterator;
  }
  if(existing_iterator == quotes->end() ||
      existing_iterator->m_quote.m_price != quote.m_quote.m_price) {
    if(quote.m_quote.m_size != 0) {
      quotes->insert(quote, find_insert_position());
    }
    return;
  }
  if(quote.m_quote.m_size == 0) {
    quotes->remove(existing_iterator);
  } else {
    auto insert_iterator = find_insert_position();
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
      quotes->set(existing_index, quote);
      quotes->move(existing_index, insert_index);
    }
  }
}

void LocalBookViewModel::update(const TimeAndSale& time_and_sale) {
  auto technicals = m_model.get_session_technicals()->get();
  Nexus::update(technicals, time_and_sale, m_market_center);
  m_model.get_session_technicals()->set(technicals);
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
  auto find_order = [&] (auto& orders) -> optional<int> {
    for(auto i = 0; i != static_cast<int>(orders.size()); ++i) {
      if(orders[i]->get_info().m_id == report.m_id) {
        return i;
      }
    }
    return none;
  };
  auto update_order = [&] (int index, auto& orders, auto& user_orders) {
    auto user_order = user_orders.get(index);
    user_order.m_status = report.m_status;
    user_order.m_size -= report.m_last_quantity;
    user_orders.set(index, user_order);
    if(is_terminal(report.m_status)) {
      m_pegged_entries.erase(report.m_id);
      orders.erase(std::next(orders.begin(), index));
      user_orders.remove(index);
    }
  };
  if(auto index = find_order(m_bid_orders)) {
    update_order(*index, m_bid_orders, *m_model.get_bid_orders());
  } else if(auto index = find_order(m_ask_orders)) {
    update_order(*index, m_ask_orders, *m_model.get_ask_orders());
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
    for(auto i = quotes.get_size() - 1; i >= 0; --i) {
      if(!quotes.get(i).m_mpid.empty()) {
        quotes.remove(i);
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
  auto price = compute_peg_price(
    entry.m_exec_inst, m_model.get_bbo_quote()->get(), fields.m_side);
  entry.m_is_initialized = price != Money::ZERO;
  entry.m_effective_price = clamp_to_limit(
    price - direction * entry.m_peg_difference, fields.m_price, fields.m_side);
  m_pegged_entries[order.get_info().m_id] = entry;
}

void LocalBookViewModel::update_pegged_orders() {
  if(m_pegged_entries.empty()) {
    return;
  }
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
      auto price = compute_peg_price(entry.m_exec_inst, bbo, side);
      if(price == Money::ZERO) {
        continue;
      }
      auto candidate = price - direction * entry.m_peg_difference;
      if(!entry.m_is_initialized ||
          direction * candidate > direction * entry.m_effective_price) {
        entry.m_effective_price = candidate;
        entry.m_is_initialized = true;
      }
      entry.m_effective_price = clamp_to_limit(
        entry.m_effective_price, order->get_info().m_fields.m_price, side);
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

const std::shared_ptr<SessionTechnicalsModel>&
    LocalBookViewModel::get_session_technicals() const {
  return m_model.get_session_technicals();
}
