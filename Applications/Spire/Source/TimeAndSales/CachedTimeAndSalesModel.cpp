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
    m_is_loading(false),
    m_source_connection(m_source->connect_update_signal(
      std::bind_front(&CachedTimeAndSalesModel::on_update, this))) {}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    CachedTimeAndSalesModel::query_until(
      Beam::Sequence sequence, int max_count) {
  if(sequence != Beam::Sequence::PRESENT) {
    return m_source->query_until(sequence, max_count);
  }
  if(!m_is_loading && static_cast<int>(m_recent.size()) >= max_count) {
    return get_recent(max_count);
  }
  auto [future, promise] = make_future<std::vector<Entry>>();
  m_pending.push_back(PendingQuery(max_count, std::move(future)));
  if(!m_is_loading) {
    m_is_loading = true;
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

void CachedTimeAndSalesModel::on_update(const Entry& entry) {
  m_recent.push_back(entry);
  m_update_signal(entry);
}

void CachedTimeAndSalesModel::on_snapshot(std::vector<Entry> snapshot) {
  m_is_loading = false;
  for(auto& entry : snapshot) {
    m_recent.push_back(entry);
  }
  auto pending = std::move(m_pending);
  m_pending.clear();
  for(auto& query : pending) {
    resolve_pending(std::move(query));
  }
}

void CachedTimeAndSalesModel::resolve_pending(PendingQuery query) {
  if(query.m_max_count <= static_cast<int>(m_recent.size())) {
    query.m_result.resolve(get_recent(query.m_max_count));
  } else if(m_recent.empty()) {
    query.m_result.resolve(std::vector<Entry>());
  } else {
    auto remaining = query.m_max_count - static_cast<int>(m_recent.size());
    // Backfill the entries strictly older than the cache. query_until is
    // inclusive of its end sequence, so passing the oldest cached sequence
    // would return that entry and duplicate it when the cache is appended.
    auto end = decrement(m_recent.front().m_time_and_sale.get_sequence());
    m_backfills.push_back(m_source->query_until(end, remaining).then(
      [this, result = std::move(query.m_result)] (auto&& older) mutable {
        auto entries = std::move(older).get();
        for(auto& entry : m_recent) {
          entries.push_back(entry);
        }
        result.resolve(std::move(entries));
      }));
  }
}
