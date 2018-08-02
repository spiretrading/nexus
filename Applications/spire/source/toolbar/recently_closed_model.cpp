#include "spire/toolbar/recently_closed_model.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

RecentlyClosedModel::RecentlyClosedModel()
    : m_next_id(0) {}

const std::vector<RecentlyClosedModel::Entry>&
    RecentlyClosedModel::get_entries() const noexcept {
  return m_entries;
}

RecentlyClosedModel::Entry RecentlyClosedModel::add(Type t,
    std::string identifier) {
  auto id = ++m_next_id;
  auto e = Entry{id, t, std::move(identifier)};
  m_entries.push_back(e);
  m_entry_added_signal(e);
  return e;
}

void RecentlyClosedModel::remove(const Entry& e) {
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

connection RecentlyClosedModel::connect_entry_added_signal(
    const EntryAddedSignal::slot_type& slot) const {
  return m_entry_added_signal.connect(slot);
}

connection RecentlyClosedModel::connect_entry_removed_signal(
    const EntryRemovedSignal::slot_type& slot) const {
  return m_entry_removed_signal.connect(slot);
}
