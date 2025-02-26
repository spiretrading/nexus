#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  auto make_preview_quote(const OrderFields& order) {
    return BookQuote("@@" + order.m_destination, false, {},
      {order.m_price, order.m_quantity, order.m_side},
      second_clock::local_time());
  }
}

BookViewModel::BookViewModel()
  : m_bids(std::make_shared<ArrayListModel<BookQuote>>()),
    m_asks(std::make_shared<ArrayListModel<BookQuote>>()),
    m_bid_orders(std::make_shared<ArrayListModel<UserOrder>>()),
    m_ask_orders(std::make_shared<ArrayListModel<UserOrder>>()),
    m_preview_order(std::make_shared<LocalValueModel<optional<OrderFields>>>()),
    m_bbo(std::make_shared<LocalBboQuoteModel>()),
    m_technicals(std::make_shared<LocalSecurityTechnicalsValueModel>()),
    m_preview_order_index(-1),
    m_preview_order_side(Side::NONE),
    m_update_connection(m_preview_order->connect_update_signal(
      std::bind_front(&BookViewModel::on_preview_order_update, this))) {}

const std::shared_ptr<ListModel<BookQuote>>& BookViewModel::get_bids() const {
  return m_bids;
}

const std::shared_ptr<ListModel<BookQuote>>& BookViewModel::get_asks() const {
  return m_asks;
}

const std::shared_ptr<ListModel<BookViewModel::UserOrder>>&
    BookViewModel::get_bid_orders() const {
  return m_bid_orders;
}

const std::shared_ptr<ListModel<BookViewModel::UserOrder>>&
    BookViewModel::get_ask_orders() const {
  return m_ask_orders;
}

const std::shared_ptr<BookViewModel::PreviewOrderModel>&
    BookViewModel::get_preview_order() const {
  return m_preview_order;
}

const std::shared_ptr<BboQuoteModel>& BookViewModel::get_bbo_quote() const {
  return m_bbo;
}

const std::shared_ptr<SecurityTechnicalsValueModel>&
    BookViewModel::get_technicals() const {
  return m_technicals;
}

void BookViewModel::on_operation(
    const ListModel<BookQuote>::Operation& operation) {
  visit(operation,
    [&] (const auto& operation) {
      m_preview_order_index.update(operation);
    });
}

void BookViewModel::on_preview_order_update(
    const optional<OrderFields>& order) {
  auto get_quotes = [&] (Side side) {
    if(side == Side::BID) {
      return get_bids();
    }
    return get_asks();
  };
  if(order) {
    auto quotes = get_quotes(order->m_side);
    if(m_preview_order_index.get_index() != -1) {
      auto quote = quotes->get(m_preview_order_index.get_index());
      quote.m_quote.m_price = order->m_price;
      quote.m_quote.m_size = order->m_quantity;
      quotes->set(m_preview_order_index.get_index(), quote);
    } else {
      m_operation_connection = quotes->connect_operation_signal(
        std::bind_front(&BookViewModel::on_operation, this));
      quotes->push(make_preview_quote(*order));
      m_preview_order_index.set(quotes->get_size() - 1);
      m_preview_order_side = order->m_side;
    }
  } else if(m_preview_order_index.get_index() != -1) {
    auto quotes = get_quotes(m_preview_order_side);
    quotes->remove(m_preview_order_index.get_index());
    m_preview_order_index.set(-1);
    m_operation_connection.disconnect();
  }
}
