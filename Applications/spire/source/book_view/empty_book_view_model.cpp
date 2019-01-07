#include "spire/book_view/empty_book_view_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

EmptyBookViewModel::EmptyBookViewModel(Security security)
    : m_security(std::move(security)) {}

const Security& EmptyBookViewModel::get_security() const {
  return m_security;
}

const BboQuote& EmptyBookViewModel::get_bbo() const {
  static BboQuote bbo;
  return bbo;
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    EmptyBookViewModel::get_asks() const {
  static auto quotes = std::vector<std::unique_ptr<BookViewModel::Quote>>();
  return quotes;
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    EmptyBookViewModel::get_bids() const {
  static auto quotes = std::vector<std::unique_ptr<BookViewModel::Quote>>();
  return quotes;
}

optional<Money> EmptyBookViewModel::get_high() const {
  return {};
}

optional<Money> EmptyBookViewModel::get_low() const {
  return {};
}

optional<Money> EmptyBookViewModel::get_open() const {
  return {};
}

optional<Money> EmptyBookViewModel::get_close() const {
  return {};
}

Quantity EmptyBookViewModel::get_volume() const {
  return 0;
}

QtPromise<void> EmptyBookViewModel::load() {
  return make_qt_promise([] {});
}

connection EmptyBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_quote_slot(
    const QuoteSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return {};
}

connection EmptyBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return {};
}
