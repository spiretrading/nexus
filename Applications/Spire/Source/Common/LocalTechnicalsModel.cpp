#include "Spire/Spire/LocalTechnicalsModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LocalTechnicalsModel::LocalTechnicalsModel(Security security)
    : m_security(std::move(security)),
      m_volume(0) {}

void LocalTechnicalsModel::set_close(Money price) {
  m_close = price;
  m_close_signal(*m_close);
}

void LocalTechnicalsModel::update(const TimeAndSale& time_and_sale) {
  m_volume += time_and_sale.m_size;
  if(!m_open) {
    m_open = time_and_sale.m_price;
    m_last_price = time_and_sale.m_price;
    m_high = time_and_sale.m_price;
    m_low = time_and_sale.m_price;
    m_open_signal(*m_open);
    m_last_price_signal(*m_last_price);
    m_high_signal(*m_high);
    m_low_signal(*m_low);
  } else if(m_last_price != time_and_sale.m_price) {
    m_last_price = time_and_sale.m_price;
    if(m_high < time_and_sale.m_price) {
      m_high = time_and_sale.m_price;
      m_high_signal(*m_high);
    } else if(m_low > time_and_sale.m_price) {
      m_low = time_and_sale.m_price;
      m_low_signal(*m_low);
    }
    m_last_price_signal(*m_last_price);
  }
  m_volume_signal(m_volume);
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
