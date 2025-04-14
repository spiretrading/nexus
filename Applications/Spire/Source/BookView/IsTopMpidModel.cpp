#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/BookView/TopMpidLevelListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

IsTopMpidModel::IsTopMpidModel(
    std::shared_ptr<TopMpidLevelListModel> top_mpid_levels,
    std::shared_ptr<MpidModel> mpid, std::shared_ptr<ValueModel<int>> level)
    : m_top_mpid_levels(std::move(top_mpid_levels)),
      m_mpid(std::move(mpid)),
      m_level(std::move(level)) {
  on_mpid(m_mpid->get());
  m_mpid_connection = m_mpid->connect_update_signal(
    std::bind_front(&IsTopMpidModel::on_mpid, this));
  m_level_connection = m_level->connect_update_signal(
    std::bind_front(&IsTopMpidModel::on_level, this));
}

const bool& IsTopMpidModel::get() const {
  return m_current.get();
}

connection IsTopMpidModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}

void IsTopMpidModel::on_mpid(const Mpid& mpid) {
  m_top_mpid = nullptr;
  if(mpid.m_origin == Mpid::Origin::BOOK_QUOTE) {
    for(auto i = 0; i != m_top_mpid_levels->get_size(); ++i) {
      if(m_top_mpid_levels->get(i).m_market == mpid.m_market) {
        m_top_mpid = make_list_value_model(m_top_mpid_levels, i);
        on_top_mpid(m_top_mpid->get());
        m_top_mpid->connect_update_signal(
          std::bind_front(&IsTopMpidModel::on_top_mpid, this));
        break;
      }
    }
  } else if(m_current.get()) {
    m_current.set(false);
  }
}

void IsTopMpidModel::on_top_mpid(const TopMpidLevel& top) {
  if(m_level->get() == top.m_level) {
    if(!m_current.get()) {
      m_current.set(true);
    }
  } else if(m_current.get()) {
    m_current.set(false);
  }
}

void IsTopMpidModel::on_level(int level) {
  if(!m_top_mpid) {
    return;
  }
  if(m_top_mpid->get().m_level == level) {
    if(!m_current.get()) {
      m_current.set(true);
    }
  } else if(m_current.get()) {
    m_current.set(false);
  }
}
