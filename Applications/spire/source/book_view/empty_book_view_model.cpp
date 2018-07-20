#include "spire/book_view/empty_book_view_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

empty_book_view_model::empty_book_view_model(Security security)
    : m_security(std::move(security)) {}

const Security& empty_book_view_model::get_security() const {
  return m_security;
}

const BboQuote& empty_book_view_model::get_bbo() const {
  static BboQuote bbo;
  return bbo;
}

Money empty_book_view_model::get_high() const {
  return Money::ZERO;
}

Money empty_book_view_model::get_low() const {
  return Money::ZERO;
}

Money empty_book_view_model::get_open() const {
  return Money::ZERO;
}

Money empty_book_view_model::get_close() const {
  return Money::ZERO;
}

Quantity empty_book_view_model::get_volume() const {
  return 0;
}

connection empty_book_view_model::connect_bbo_slot(
    const bbo_signal::slot_type& slot) const {
  return {};
}

connection empty_book_view_model::connect_high_slot(
    const price_signal::slot_type& slot) const {
  return {};
}

connection empty_book_view_model::connect_low_slot(
    const price_signal::slot_type& slot) const {
  return {};
}

connection empty_book_view_model::connect_open_slot(
    const price_signal::slot_type& slot) const {
  return {};
}

connection empty_book_view_model::connect_close_slot(
    const price_signal::slot_type& slot) const {
  return {};
}

connection empty_book_view_model::connect_volume_slot(
    const quantity_signal::slot_type& slot) const {
  return {};
}
