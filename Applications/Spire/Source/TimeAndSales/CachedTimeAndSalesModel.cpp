#include "Spire/TimeAndSales/CachedTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CachedTimeAndSalesModel::CachedTimeAndSalesModel(
  std::shared_ptr<TimeAndSalesModel> source)
  : CachedTimeAndSalesModel(std::move(source), DEFAULT_COUNT) {}

CachedTimeAndSalesModel::CachedTimeAndSalesModel(
  std::shared_ptr<TimeAndSalesModel> source, int count)
  : m_source(std::move(source)),
    m_recent(count),
    m_state(State::UNLOADED),
    m_source_connection(m_source->connect_update_signal(
      std::bind_front(&CachedTimeAndSalesModel::on_update, this))) {}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    CachedTimeAndSalesModel::query_until(
      Beam::Sequence sequence, int max_count) {
  if(sequence != Beam::Sequence::PRESENT) {
    return m_source->query_until(sequence, max_count);
  }
  if(m_state != State::LOADING &&
      static_cast<int>(m_recent.size()) >= max_count) {
    return get_recent(max_count);
  }
  if(m_state == State::LOADED) {
    if(m_recent.empty() || !m_recent.full()) {
      return get_recent(static_cast<int>(m_recent.size()));
    }
    return load_older(max_count);
  }
  auto [future, promise] = make_future<std::vector<Entry>>();
  m_pending.push_back(PendingQuery(max_count, std::move(future)));
  if(m_state == State::UNLOADED) {
    m_state = State::LOADING;
    m_backfills.clear();
    m_load = m_source->query_until(
      Beam::Sequence::PRESENT, static_cast<int>(m_recent.capacity())).then(
        [this] (auto&& result) {
          on_snapshot(std::move(result).get());
        });
  }
  return std::move(promise);
}

connection CachedTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

std::vector<TimeAndSalesModel::Entry>
    CachedTimeAndSalesModel::get_recent(int max_count) const {
  return std::vector<Entry>(m_recent.end() - max_count, m_recent.end());
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    CachedTimeAndSalesModel::load_older(int max_count) {
  auto remaining = max_count - static_cast<int>(m_recent.size());
  auto end = decrement(m_recent.front().m_time_and_sale.get_sequence());
  auto recent = std::vector<Entry>(m_recent.begin(), m_recent.end());
  return m_source->query_until(end, remaining).then(
    [recent = std::move(recent)] (auto&& older) mutable {
      auto entries = std::move(older).get();
      for(auto& entry : recent) {
        entries.push_back(std::move(entry));
      }
      return entries;
    });
}

void CachedTimeAndSalesModel::on_update(const Entry& entry) {
  if(m_state == State::LOADING) {
    m_pending_updates.push_back(entry);
  } else {
    m_recent.push_back(entry);
  }
  m_update_signal(entry);
}

void CachedTimeAndSalesModel::on_snapshot(std::vector<Entry> snapshot) {
  m_state = State::LOADED;
  m_recent.clear();
  for(auto& entry : snapshot) {
    m_recent.push_back(std::move(entry));
  }
  for(auto& entry : m_pending_updates) {
    if(m_recent.empty() || entry.m_time_and_sale.get_sequence() >
        m_recent.back().m_time_and_sale.get_sequence()) {
      m_recent.push_back(entry);
    }
  }
  m_pending_updates.clear();
  auto pending = std::move(m_pending);
  m_pending.clear();
  for(auto& query : pending) {
    resolve_pending(std::move(query));
  }
}

void CachedTimeAndSalesModel::resolve_pending(PendingQuery query) {
  if(query.m_max_count <= static_cast<int>(m_recent.size())) {
    query.m_result.resolve(get_recent(query.m_max_count));
  } else if(m_recent.empty() || !m_recent.full()) {
    query.m_result.resolve(get_recent(static_cast<int>(m_recent.size())));
  } else {
    m_backfills.push_back(load_older(query.m_max_count).then(
      [result = std::move(query.m_result)] (auto&& entries) mutable {
        result.resolve(std::move(entries).get());
      }));
  }
}
