#include "spire/spire/local_technicals_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LocalTechnicalsModel::LocalTechnicalsModel(Security security)
    : m_security(std::move(security)),
      m_volume(0),
      m_high_changed(false),
      m_low_changed(false),
      m_open_changed(false),
      m_last_price_changed(false) {
  m_volume_signal(m_volume);
}

void LocalTechnicalsModel::set_close(Money price) {
  m_close = price;
  m_close_signal(*m_close);
}

void LocalTechnicalsModel::update(const TimeAndSale& time_and_sale) {
  if(!m_open.is_initialized()) {
    m_open = time_and_sale.m_price;
    m_open_changed = true;
  }
  if(time_and_sale.m_price != m_last_price) {
    m_last_price = time_and_sale.m_price;
    m_last_price_changed = true;
  }
  if(!m_high.is_initialized() || time_and_sale.m_price > *m_high) {
    m_high = time_and_sale.m_price;
    m_high_changed = true;
  }
  if(!m_low.is_initialized() || time_and_sale.m_price < *m_low) {
    m_low = time_and_sale.m_price;
    m_low_changed = true;
  }
  m_volume += time_and_sale.m_size;
  m_volume_signal(m_volume);
  if(m_open_changed) {
    m_open_signal(*m_open);
    m_open_changed = false;
  }
  if(m_high_changed) {
    m_high_signal(*m_high);
    m_high_changed = false;
  }
  if(m_low_changed) {
    m_low_signal(*m_low);
    m_low_changed = false;
  }
  if(m_last_price_changed) {
    m_last_price_signal(*m_last_price);
    m_last_price_changed = false;
  }
}

const Security& LocalTechnicalsModel::get_security() const {
  return m_security;
}

optional<Money> LocalTechnicalsModel::get_high() const {
  return m_high;
}

optional<Money> LocalTechnicalsModel::get_low() const {
  return m_low;
}

optional<Money> LocalTechnicalsModel::get_open() const {
  return m_open;
}

optional<Money> LocalTechnicalsModel::get_close() const {
  return m_close;
}

optional<Money> LocalTechnicalsModel::get_last_price() const {
  return m_last_price;
}

Quantity LocalTechnicalsModel::get_volume() const {
  return m_volume;
}

QtPromise<void> LocalTechnicalsModel::load() {
  return QtPromise<void>([] {});
}

connection LocalTechnicalsModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_high_signal.connect(slot);
}

connection LocalTechnicalsModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_low_signal.connect(slot);
}

connection LocalTechnicalsModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection LocalTechnicalsModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_close_signal.connect(slot);
}

connection LocalTechnicalsModel::connect_last_price_slot(
    const PriceSignal::slot_type& slot) const {
  return m_last_price_signal.connect(slot);
}

connection LocalTechnicalsModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}
