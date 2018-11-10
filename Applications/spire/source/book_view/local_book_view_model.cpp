#include "spire/book_view/local_book_view_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LocalBookViewModel::LocalBookViewModel(Security security)
    : m_security(std::move(security)) {}

void LocalBookViewModel::update(const BboQuote& bbo) {
  m_bbo = bbo;
  m_bbo_signal(m_bbo);
}

const Security& LocalBookViewModel::get_security() const {
  return m_security;
}

const BboQuote& LocalBookViewModel::get_bbo() const {
  return m_bbo;
}

const std::vector<BookQuote>& LocalBookViewModel::get_asks() const {
  return m_asks;
}

const std::vector<BookQuote>& LocalBookViewModel::get_bids() const {
  return m_bids;
}

optional<Money> LocalBookViewModel::get_high() const {
  return m_high;
}

optional<Money> LocalBookViewModel::get_low() const {
  return m_low;
}

optional<Money> LocalBookViewModel::get_open() const {
  return m_open;
}

optional<Money> LocalBookViewModel::get_close() const {
  return m_close;
}

Quantity LocalBookViewModel::get_volume() const {
  return m_volume;
}

QtPromise<void> LocalBookViewModel::load() {
  return make_qt_promise([] {});
}

connection LocalBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_bbo_signal.connect(slot);
}

connection LocalBookViewModel::connect_book_quote_slot(
    const BookQuoteSignal::slot_type& slot) const {
  return m_book_quote_signal.connect(slot);
}

connection LocalBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_high_signal.connect(slot);
}

connection LocalBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_low_signal.connect(slot);
}

connection LocalBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection LocalBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_close_signal.connect(slot);
}

connection LocalBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}
