#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

BookViewModel::BookViewModel()
  : m_bids(std::make_shared<ArrayListModel<BookQuote>>()),
    m_asks(std::make_shared<ArrayListModel<BookQuote>>()),
    m_bid_orders(std::make_shared<ArrayListModel<UserOrder>>()),
    m_ask_orders(std::make_shared<ArrayListModel<UserOrder>>()),
    m_preview_order(std::make_shared<LocalValueModel<optional<OrderFields>>>()),
    m_bbo(std::make_shared<LocalBboQuoteModel>()),
    m_technicals(std::make_shared<LocalSecurityTechnicalsValueModel>()) {}

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
