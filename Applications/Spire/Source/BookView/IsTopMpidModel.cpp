#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

IsTopMpidModel::IsTopMpidModel(
    std::shared_ptr<ListModel<TopMpidPrice>> top_mpid_prices,
    std::shared_ptr<BookEntryModel> mpid,
    std::shared_ptr<ValueModel<Money>> price)
    : m_top_mpid_prices(std::move(top_mpid_prices)),
      m_mpid(std::move(mpid)),
      m_price(std::move(price)) {
  on_mpid(m_mpid->get());
  if(!m_top_mpid) {
    m_top_mpid_prices_connection = m_top_mpid_prices->connect_operation_signal(
      std::bind_front(&IsTopMpidModel::on_operation, this));
  }
  m_mpid_connection = m_mpid->connect_update_signal(
    std::bind_front(&IsTopMpidModel::on_mpid, this));
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

void IsTopMpidModel::initialize_top_mpid() {
  for(auto i = 0; i != m_top_mpid_prices->get_size(); ++i) {
    if(m_top_mpid_prices->get(i).m_venue == m_venue) {
      m_top_mpid = make_list_value_model(m_top_mpid_prices, i);
      on_top_mpid(m_top_mpid->get());
      m_top_mpid->connect_update_signal(
        std::bind_front(&IsTopMpidModel::on_top_mpid, this));
      break;
    }
  }
}

void IsTopMpidModel::on_mpid(const BookEntry& mpid) {
  m_top_mpid = nullptr;
  if(auto quote = boost::get<BookQuote>(&mpid)) {
    m_venue = quote->m_venue;
    initialize_top_mpid();
  } else if(m_current.get()) {
    m_venue = Venue();
    m_current.set(false);
  }
}

void IsTopMpidModel::on_top_mpid(const TopMpidPrice& top) {
  if(m_price->get() == top.m_price) {
    if(!m_current.get()) {
      m_current.set(true);
    }
  } else if(m_current.get()) {
    m_current.set(false);
  }
}

void IsTopMpidModel::on_price(Money price) {
  if(!m_top_mpid) {
    return;
  }
  if(m_top_mpid->get().m_price == price) {
    if(!m_current.get()) {
      m_current.set(true);
    }
  } else if(m_current.get()) {
    m_current.set(false);
  }
}

void IsTopMpidModel::on_operation(
    const ListModel<TopMpidPrice>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TopMpidPrice>::AddOperation& operation) {
      if(m_top_mpid_prices->get(operation.m_index).m_venue == m_venue) {
        initialize_top_mpid();
        m_top_mpid_prices_connection.disconnect();
      }
    });
}
