#include "Spire/BookView/IsTopMpidModel.hpp"

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
      m_price(std::move(price)),
      m_is_top_mpid_removed(false) {
  on_mpid(m_mpid->get());
  m_top_mpid_prices_connection = m_top_mpid_prices->connect_operation_signal(
    std::bind_front(&IsTopMpidModel::on_operation, this));
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

void IsTopMpidModel::initialize_top_price() {
  m_top_price = none;
  for(auto i = 0; i != m_top_mpid_prices->get_size(); ++i) {
    auto& top = m_top_mpid_prices->get(i);
    if(top.m_venue == m_venue) {
      m_top_price = top.m_price;
      break;
    }
  }
  update_current();
}

void IsTopMpidModel::update_top_price(int index) {
  auto& top = m_top_mpid_prices->get(index);
  if(top.m_venue != m_venue) {
    return;
  }
  m_top_price = top.m_price;
  update_current();
}

void IsTopMpidModel::update_current() {
  auto is_top = m_top_price && *m_top_price == m_price->get();
  if(is_top != m_current.get()) {
    m_current.set(is_top);
  }
}

void IsTopMpidModel::on_mpid(const BookEntry& mpid) {
  if(auto quote = boost::get<BookQuote>(&mpid)) {
    if(quote->m_venue == m_venue) {
      return;
    }
    m_venue = quote->m_venue;
    initialize_top_price();
  } else {
    m_top_price = none;
    m_venue = Venue();
    update_current();
  }
}

void IsTopMpidModel::on_price(Money price) {
  update_current();
}

void IsTopMpidModel::on_operation(
    const ListModel<TopMpidPrice>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TopMpidPrice>::AddOperation& operation) {
      update_top_price(operation.m_index);
    },
    [&] (const ListModel<TopMpidPrice>::UpdateOperation& operation) {
      update_top_price(operation.m_index);
    },
    [&] (const ListModel<TopMpidPrice>::PreRemoveOperation& operation) {
      m_is_top_mpid_removed =
        m_top_mpid_prices->get(operation.m_index).m_venue == m_venue;
    },
    [&] (const ListModel<TopMpidPrice>::RemoveOperation& operation) {
      if(m_is_top_mpid_removed) {
        initialize_top_price();
      }
    });
}
