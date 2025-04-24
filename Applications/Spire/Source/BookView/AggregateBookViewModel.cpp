#include "Spire/BookView/AggregateBookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

AggregateBookViewModel::AggregateBookViewModel(
  std::shared_ptr<BookQuoteListModel> bids,
  std::shared_ptr<BookQuoteListModel> asks,
  std::shared_ptr<UserOrderListModel> bid_orders,
  std::shared_ptr<UserOrderListModel> ask_orders,
  std::shared_ptr<PreviewOrderModel> preview_order,
  std::shared_ptr<BboQuoteModel> bbo_quote,
  std::shared_ptr<SecurityTechnicalsValueModel> technicals)
  : m_bids(std::move(bids)),
    m_asks(std::move(asks)),
    m_bid_orders(std::move(bid_orders)),
    m_ask_orders(std::move(ask_orders)),
    m_preview_order(std::move(preview_order)),
    m_bbo_quote(std::move(bbo_quote)),
    m_technicals(std::move(technicals)) {}

const std::shared_ptr<BookQuoteListModel>&
    AggregateBookViewModel::get_bids() const {
  return m_bids;
}

const std::shared_ptr<BookQuoteListModel>&
    AggregateBookViewModel::get_asks() const {
  return m_asks;
}

const std::shared_ptr<AggregateBookViewModel::UserOrderListModel>&
    AggregateBookViewModel::get_bid_orders() const {
  return m_bid_orders;
}

const std::shared_ptr<AggregateBookViewModel::UserOrderListModel>&
    AggregateBookViewModel::get_ask_orders() const {
  return m_ask_orders;
}

const std::shared_ptr<AggregateBookViewModel::PreviewOrderModel>&
    AggregateBookViewModel::get_preview_order() const {
  return m_preview_order;
}

const std::shared_ptr<BboQuoteModel>&
    AggregateBookViewModel::get_bbo_quote() const {
  return m_bbo_quote;
}

const std::shared_ptr<SecurityTechnicalsValueModel>&
    AggregateBookViewModel::get_technicals() const {
  return m_technicals;
}

std::shared_ptr<AggregateBookViewModel>
    Spire::make_local_aggregate_book_view_model() {
  return std::make_shared<AggregateBookViewModel>(
    std::make_shared<ArrayListModel<BookQuote>>(),
    std::make_shared<ArrayListModel<BookQuote>>(),
    std::make_shared<ArrayListModel<AggregateBookViewModel::UserOrder>>(),
    std::make_shared<ArrayListModel<AggregateBookViewModel::UserOrder>>(),
    std::make_shared<LocalValueModel<optional<OrderFields>>>(),
    std::make_shared<LocalBboQuoteModel>(),
    std::make_shared<LocalSecurityTechnicalsValueModel>());
}
