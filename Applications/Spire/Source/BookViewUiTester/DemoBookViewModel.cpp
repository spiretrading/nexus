#include "Spire/BookViewUiTester/DemoBookViewModel.hpp"
#include <ranges>
#include <QRandomGenerator>
#include "Spire/Spire/ListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - ptime(gregorian::date(1970, 1, 1))).total_milliseconds();
  }

  auto get_quotes(const BookViewModel& model, Side side) {
    if(side == Side::BID) {
      return model.get_bids();
    }
    return model.get_asks();
  }

  auto get_orders(const BookViewModel& model, Side side) {
    if(side == Side::BID) {
      return model.get_bid_orders();
    }
    return model.get_ask_orders();
  }

  int find_book_quote(const ListModel<BookQuote>& quotes,
      const optional<OrderFields>& preview_order,
      const std::string& mpid, const Money& price) {
    auto i = std::find_if(quotes.begin(), quotes.end(),
      [&] (const BookQuote& quote) {
        return quote.m_mpid == mpid && quote.m_quote.m_price == price;
      });
    if(i == quotes.end()) {
      return -1;
    }
    return std::distance(quotes.begin(), i);
  }

  int find_order(const ListModel<BookViewModel::UserOrder>& orders,
      const Destination& destination, const Money& price) {
    auto i = std::find_if(orders.begin(), orders.end(),
      [&] (const BookViewModel::UserOrder& order) {
        return order.m_destination == destination && order.m_price == price;
      });
    if(i == orders.end()) {
      return -1;
    }
    return std::distance(orders.begin(), i);
  }

  auto make_user_order(const DemoBookViewModel::OrderInfo& order) {
    return BookViewModel::UserOrder(order.m_order_fields.m_destination,
      order.m_order_fields.m_price, order.m_order_fields.m_quantity,
      order.m_status);
  }
}

DemoBookViewModel::DemoBookViewModel(std::shared_ptr<BookViewModel> model)
  : m_model(std::move(model)),
    m_bbo(BboQuote{Quote(Money(0), 0, Side::BID),
      Quote{Money(std::numeric_limits<Quantity>::max()), 0, Side::ASK},
      second_clock::universal_time()}),
BEAM_SUPPRESS_THIS_INITIALIZER()
    m_bid_operation_connection(m_model->get_bids()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_bid_operation, this))),
    m_ask_operation_connection(m_model->get_asks()->connect_operation_signal(
      std::bind_front(&DemoBookViewModel::on_ask_operation, this))) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

const std::shared_ptr<BookViewModel>& DemoBookViewModel::get_model() const {
  return m_model;
}

void DemoBookViewModel::submit_book_quote(const BookQuote& quote) {
  auto quotes = get_quotes(*m_model, quote.m_quote.m_side);
  auto i = find_book_quote(*quotes, m_model->get_preview_order()->get(),
    quote.m_mpid, quote.m_quote.m_price);
  if(i >= 0) {
    if(quote.m_quote.m_size == 0) {
      quotes->remove(i);
    } else {
      quotes->set(i, quote);
    }
  } else if(quote.m_quote.m_size != 0) {
    quotes->push(quote);
  }
}

void DemoBookViewModel::submit_order(const OrderInfo& order_info) {
  auto orders = get_orders(*m_model, order_info.m_order_fields.m_side);
  auto order_index = find_order(*orders,
    order_info.m_order_fields.m_destination, order_info.m_order_fields.m_price);
  if(order_index >= 0) {
    if(order_info.m_status == OrderStatus::NEW) {
      auto order = orders->get(order_index);
      order.m_size += order_info.m_order_fields.m_quantity;
      order.m_status = order_info.m_status;
      orders->set(order_index, order);
    } else {
      auto order = orders->get(order_index);
      order.m_size -= order_info.m_order_fields.m_quantity;
      order.m_status = order_info.m_status;
      orders->set(order_index, order);
      if(order.m_size <= 0) {
        orders->remove(order_index);
      }
    }
  } else if(order_info.m_status == OrderStatus::NEW) {
    orders->push(make_user_order(order_info));
  }
}

void DemoBookViewModel::on_bid_operation(
    const ListModel<BookQuote>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<BookQuote>::AddOperation& operation) {
      auto& quote = m_model->get_bids()->get(operation.m_index);
      if(m_bbo.m_bid.m_price < quote.m_quote.m_price) {
        m_bbo.m_bid = quote.m_quote;
        m_model->get_bbo_quote()->set(m_bbo);
      }
    },
    [&] (const ListModel<BookQuote>::PreRemoveOperation& operation) {
      auto& bids = m_model->get_bids();
      auto& quote = bids->get(operation.m_index);
      if(m_bbo.m_bid.m_price == quote.m_quote.m_price) {
        auto max_price = Money(0);
        auto max_price_index = -1;
        for(auto i = 0; i < bids->get_size(); ++i) {
          auto& price = bids->get(i).m_quote.m_price;
          if(price < m_bbo.m_bid.m_price && price > max_price) {
            max_price = price;
            max_price_index = i;
          }
        }
        if(max_price_index >= 0) {
          m_bbo.m_bid = bids->get(max_price_index).m_quote;
        } else {
          m_bbo.m_bid = Quote{max_price, 0, Side::BID};
        }
        m_model->get_bbo_quote()->set(m_bbo);
      }
    });
}

void DemoBookViewModel::on_ask_operation(
    const ListModel<BookQuote>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<BookQuote>::AddOperation& operation) {
      auto& quote = m_model->get_asks()->get(operation.m_index);
      if(m_bbo.m_ask.m_price > quote.m_quote.m_price) {
        m_bbo.m_ask = quote.m_quote;
        m_model->get_bbo_quote()->set(m_bbo);
      }
    },
    [&] (const ListModel<BookQuote>::PreRemoveOperation& operation) {
      auto& asks = m_model->get_asks();
      auto& quote = asks->get(operation.m_index);
      if(m_bbo.m_ask.m_price == quote.m_quote.m_price) {
        auto min_price = Money(std::numeric_limits<Quantity>::max());
        auto min_price_index = -1;
        for(auto i = 0; i < asks->get_size(); ++i) {
          auto& price = asks->get(i).m_quote.m_price;
          if(price > m_bbo.m_ask.m_price && price < min_price) {
            min_price = price;
            min_price_index = i;
          }
        }
        if(min_price_index >= 0) {
          m_bbo.m_ask = asks->get(min_price_index).m_quote;
        } else {
          m_bbo.m_ask = Quote{min_price, 0, Side::ASK};
        }
        m_model->get_bbo_quote()->set(m_bbo);
      }
    });
}
