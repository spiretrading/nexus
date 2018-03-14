#include "spire/toolbar/recently_closed_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

recently_closed_model::recently_closed_model()
    : m_next_id(0) {}

const std::vector<recently_closed_model::entry>&
    recently_closed_model::get_entries() const noexcept {
  return m_entries;
}

recently_closed_model::entry recently_closed_model::add(type t,
    std::string identifier) {
  auto id = ++m_next_id;
  auto e = entry{id, t, std::move(identifier)};
  m_entries.push_back(e);
  m_entry_added_signal(e);
  return e;
}

void recently_closed_model::remove(const entry& e) {
  auto i = std::find_if(m_entries.begin(), m_entries.end(),
    [&] (auto& i) {
      return i.m_id == e.m_id;
    });
  if(i == m_entries.end()) {
    return;
  }
  auto deleted_entry = *i;
  m_entries.erase(i);
  m_entry_removed_signal(deleted_entry);
}

connection recently_closed_model::connect_entry_added_signal(
    const entry_added_signal::slot_type& slot) const {
  return m_entry_added_signal.connect(slot);
}

connection recently_closed_model::connect_entry_removed_signal(
    const entry_removed_signal::slot_type& slot) const {
  return m_entry_removed_signal.connect(slot);
}
