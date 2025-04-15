#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/BookView/TopMpidLevelListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

IsTopMpidModel::IsTopMpidModel(
    std::shared_ptr<ListModel<TopMpidLevel>> top_mpid_levels,
    std::shared_ptr<MpidModel> mpid, std::shared_ptr<ValueModel<Money>> price)
    : m_top_mpid_levels(std::move(top_mpid_levels)),
      m_mpid(std::move(mpid)),
      m_price(std::move(price)) {
  on_mpid(m_mpid->get());
  if(!m_top_mpid) {
    m_top_mpid_levels_connection = m_top_mpid_levels->connect_operation_signal(
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
  for(auto i = 0; i != m_top_mpid_levels->get_size(); ++i) {
    if(m_top_mpid_levels->get(i).m_market == m_market) {
      m_top_mpid = make_list_value_model(m_top_mpid_levels, i);
      on_top_mpid(m_top_mpid->get());
      m_top_mpid->connect_update_signal(
        std::bind_front(&IsTopMpidModel::on_top_mpid, this));
      break;
    }
  }
}

void IsTopMpidModel::on_mpid(const Mpid& mpid) {
  m_top_mpid = nullptr;
  if(mpid.m_origin == Mpid::Origin::BOOK_QUOTE) {
    m_market = mpid.m_market;
    initialize_top_mpid();
  } else if(m_current.get()) {
    m_market = MarketCode();
    m_current.set(false);
  }
}

void IsTopMpidModel::on_top_mpid(const TopMpidLevel& top) {
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
    const ListModel<TopMpidLevel>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TopMpidLevel>::AddOperation& operation) {
      if(m_top_mpid_levels->get(operation.m_index).m_market == m_market) {
        initialize_top_mpid();
        m_top_mpid_levels_connection.disconnect();
      }
    });
}
