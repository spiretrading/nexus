#include "Spire/BookView/IsTopMpidModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

IsTopMpidModel::IsTopMpidModel(
    std::shared_ptr<TopMpidPriceListModel> top_mpid_prices,
    std::shared_ptr<BookEntryModel> entry,
    std::shared_ptr<ValueModel<Money>> price)
    : m_top_mpid_prices(std::move(top_mpid_prices)),
      m_entry(std::move(entry)),
      m_price(std::move(price)) {
  on_entry(m_entry->get());
  m_entry_connection = m_entry->connect_update_signal(
    std::bind_front(&IsTopMpidModel::on_entry, this));
  m_price_connection = m_price->connect_update_signal(
    std::bind_front(&IsTopMpidModel::on_price, this));
}

const bool& IsTopMpidModel::get() const {
  return m_current.get();
}

connection IsTopMpidModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}

void IsTopMpidModel::set_venue(Venue venue) {
  if(venue == m_venue) {
    return;
  }
  m_venue = venue;
  if(m_venue) {
    m_top_price = m_top_mpid_prices->get_top_price(m_venue);
    m_top_price_connection = m_top_price->connect_update_signal(
      std::bind_front(&IsTopMpidModel::on_top_price, this));
  } else {
    m_top_price_connection.disconnect();
    m_top_price = nullptr;
  }
  update_current();
}

void IsTopMpidModel::update_current() {
  auto is_top = m_top_price && m_top_price->get() &&
    *m_top_price->get() == m_price->get();
  if(is_top != m_current.get()) {
    m_current.set(is_top);
  }
}

void IsTopMpidModel::on_entry(const BookEntry& entry) {
  if(auto quote = boost::get<BookQuote>(&entry)) {
    set_venue(quote->m_venue);
  } else {
    set_venue(Venue());
  }
}

void IsTopMpidModel::on_price(Money price) {
  update_current();
}

void IsTopMpidModel::on_top_price(const optional<Money>& price) {
  update_current();
}
