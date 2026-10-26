#include "Spire/TimeAndSales/ReplayTimeAndSalesModel.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/TimeAndSales/LoadTimeAndSales.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto READ_AHEAD = seconds(30);
  const auto READ_SIZE = 1000;
  const auto LOW_WATER = 200;
  const auto MAX_LOW_WATER = 5000;
  const auto JUMP_THRESHOLD = seconds(2);
  const auto MIN_READ_INTERVAL = milliseconds(250);
  const auto DEFAULT_PAGE_SIZE = 50;
  const auto SEEK_INTERVAL_MS = 100;

  struct Chunk {
    std::vector<TimeAndSalesModel::Type> m_entries;
    optional<SequencedBboQuote> m_bbo;
    optional<Beam::Sequence> m_anchor;
  };

  Decimal get_speed_factor(const ReplaySession& session) {
    return std::max<Decimal>(Decimal(1), session.get_speed()->get());
  }

  time_duration scale_duration(time_duration duration, const Decimal& factor) {
    return milliseconds((Decimal(duration.total_milliseconds()) *
      factor).convert_to<std::int64_t>());
  }

  Chunk read_chunk(MarketDataClient client, const Ticker& ticker,
      const Range& range, const SnapshotLimit& limit,
      optional<SequencedBboQuote> bbo) {
    auto chunk = Chunk();
    for(auto& time_and_sale :
        load_time_and_sales(client, ticker, range, limit)) {
      chunk.m_entries.push_back(TimeAndSalesModel::Type(
        std::move(time_and_sale), BboIndicator::UNKNOWN));
    }
    chunk.m_bbo =
      load_bbo_indicators(client, ticker, chunk.m_entries, std::move(bbo));
    return chunk;
  }

  Beam::Sequence load_anchor(
      MarketDataClient client, const Ticker& ticker, ptime end) {
    auto last = load_time_and_sales(client, ticker,
      Range(Beam::Sequence::FIRST, end), SnapshotLimit::from_tail(1));
    if(last.empty()) {
      return Beam::Sequence::FIRST;
    }
    return increment(last.back().get_sequence());
  }
}

ReplayTimeAndSalesModel::ReplayTimeAndSalesModel(Ticker ticker,
    MarketDataClient client, std::shared_ptr<ReplaySession> session)
    : m_ticker(std::move(ticker)),
      m_client(std::move(client)),
      m_session(std::move(session)),
      m_time_client(m_session->get_time_client()),
      m_position(m_session->get_position()->get()),
      m_read_end(std::min(m_position, m_time_client.get_time())),
      m_last_read_time(neg_infin),
      m_page_size(DEFAULT_PAGE_SIZE),
      m_read_generation(0),
      m_older_generation(0),
      m_is_reading(false),
      m_is_seeking(false),
      m_is_seek_querying(false) {
  m_seek_timer.setSingleShot(true);
  m_seek_timer.setInterval(SEEK_INTERVAL_MS);
  QObject::connect(&m_seek_timer, &QTimer::timeout,
    std::bind_front(&ReplayTimeAndSalesModel::on_seek_timeout, this));
  m_timeline_connection = m_session->get_timeline()->connect_update_signal(
    std::bind_front(&ReplayTimeAndSalesModel::on_timeline, this));
  m_position_connection = m_session->get_position()->connect_update_signal(
    std::bind_front(&ReplayTimeAndSalesModel::on_position, this));
  m_speed_connection = m_session->get_speed()->connect_update_signal(
    std::bind_front(&ReplayTimeAndSalesModel::on_speed, this));
  m_state_connection = m_session->get_state()->connect_update_signal(
    std::bind_front(&ReplayTimeAndSalesModel::on_state, this));
  reconcile();
}

QtPromise<void> ReplayTimeAndSalesModel::load_older(int max_count) {
  if(max_count > 0) {
    m_page_size = max_count;
  }
  auto end = [&] () -> Range::Point {
    if(m_entries.empty()) {
      return std::min(m_position, m_read_end);
    }
    return decrement(m_entries.front().m_time_and_sale.get_sequence());
  }();
  auto generation = m_older_generation;
  return QtPromise([=, client = m_client, ticker = m_ticker,
      range = Range(get_start(), end)] {
    return read_chunk(client, ticker, range,
      SnapshotLimit::from_tail(max_count), none).m_entries;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    try {
      on_older(generation, std::move(result).get());
    } catch(const std::exception&) {}
  });
}

int ReplayTimeAndSalesModel::get_size() const {
  return static_cast<int>(m_entries.size());
}

const ReplayTimeAndSalesModel::Type&
    ReplayTimeAndSalesModel::get(int index) const {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_entries[index];
}

connection ReplayTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void ReplayTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

const ptime& ReplayTimeAndSalesModel::get_start() const {
  return m_session->get_timeline()->get().m_start;
}

Range::Point ReplayTimeAndSalesModel::get_cursor() const {
  if(!m_buffer.empty()) {
    return increment(m_buffer.back().m_time_and_sale.get_sequence());
  } else if(!m_entries.empty()) {
    return increment(m_entries.back().m_time_and_sale.get_sequence());
  } else if(m_anchor) {
    return *m_anchor;
  }
  return m_read_end;
}

time_duration ReplayTimeAndSalesModel::get_read_ahead() const {
  return scale_duration(READ_AHEAD, get_speed_factor(*m_session));
}

time_duration ReplayTimeAndSalesModel::get_jump_threshold() const {
  return scale_duration(JUMP_THRESHOLD, get_speed_factor(*m_session));
}

int ReplayTimeAndSalesModel::get_low_water() const {
  return std::min<Decimal>(Decimal(MAX_LOW_WATER),
    Decimal(LOW_WATER) * get_speed_factor(*m_session)).convert_to<int>();
}

void ReplayTimeAndSalesModel::append(Type entry) {
  if(entry.m_time_and_sale->m_timestamp < get_start()) {
    return;
  }
  if(!m_entries.empty() && entry.m_time_and_sale.get_sequence() <=
      m_entries.back().m_time_and_sale.get_sequence()) {
    return;
  }
  m_entries.push_back(std::move(entry));
  m_transaction.push(AddOperation(get_size() - 1));
}

void ReplayTimeAndSalesModel::pop_front() {
  m_transaction.transact([&] {
    m_transaction.push(PreRemoveOperation(0));
    m_entries.pop_front();
    m_transaction.push(RemoveOperation(0));
  });
}

void ReplayTimeAndSalesModel::pop_back() {
  auto index = get_size() - 1;
  m_transaction.transact([&] {
    m_transaction.push(PreRemoveOperation(index));
    m_entries.pop_back();
    m_transaction.push(RemoveOperation(index));
  });
}

void ReplayTimeAndSalesModel::replace(std::vector<Type> entries) {
  ++m_older_generation;
  m_transaction.transact([&] {
    while(!m_entries.empty()) {
      pop_back();
    }
    for(auto& entry : entries) {
      append(std::move(entry));
    }
  });
}

bool ReplayTimeAndSalesModel::merge(std::vector<Type>& entries) {
  if(m_entries.empty() || entries.empty()) {
    return false;
  }
  auto by_sequence = [] (const auto& entry, const auto& sequence) {
    return entry.m_time_and_sale.get_sequence() < sequence;
  };
  auto back = m_entries.back().m_time_and_sale.get_sequence();
  auto last =
    std::lower_bound(entries.begin(), entries.end(), back, by_sequence);
  if(last == entries.end() || last->m_time_and_sale.get_sequence() != back) {
    return false;
  }
  auto first = std::lower_bound(entries.begin(), last,
    m_entries.front().m_time_and_sale.get_sequence(), by_sequence);
  m_transaction.transact([&] {
    for(auto i = first; i != entries.begin(); --i) {
      m_entries.push_front(std::move(*std::prev(i)));
      m_transaction.push(AddOperation(0));
    }
    for(auto i = std::next(last); i != entries.end(); ++i) {
      append(std::move(*i));
    }
  });
  return true;
}

void ReplayTimeAndSalesModel::publish() {
  if(m_is_seeking) {
    return;
  }
  m_transaction.transact([&] {
    while(!m_buffer.empty() &&
        m_buffer.front().m_time_and_sale->m_timestamp <= m_position) {
      append(std::move(m_buffer.front()));
      m_buffer.pop_front();
    }
  });
}

void ReplayTimeAndSalesModel::read() {
  if(m_is_reading || m_is_seeking || m_live_handler) {
    return;
  }
  auto state = m_session->get_state()->get();
  auto now = m_time_client.get_time();
  auto target = [&] {
    if(state == PlaybackState::REAL_TIME) {
      return now;
    } else if(state == PlaybackState::REPLAYING) {
      return m_position + get_read_ahead();
    }
    return m_position;
  }();
  if(state != PlaybackState::REAL_TIME) {
    if(target > now) {
      if(now - m_last_read_time < MIN_READ_INTERVAL) {
        return;
      }
      target = now;
    }
    if(m_read_end >= target || std::ssize(m_buffer) >= get_low_water()) {
      return;
    }
    if(target == now) {
      m_last_read_time = now;
    }
  }
  m_is_reading = true;
  auto generation = m_read_generation;
  auto is_anchored = !m_buffer.empty() || !m_entries.empty() || m_anchor;
  m_read_promise = QtPromise([=, client = m_client, ticker = m_ticker,
      cursor = get_cursor(), anchor_end = m_read_end, bbo = m_bbo] {
    if(is_anchored) {
      return read_chunk(client, ticker, Range(cursor, target),
        SnapshotLimit::from_head(READ_SIZE), bbo);
    }
    auto anchor = load_anchor(client, ticker, anchor_end);
    auto chunk = read_chunk(client, ticker, Range(anchor, target),
      SnapshotLimit::from_head(READ_SIZE), bbo);
    chunk.m_anchor = anchor;
    return chunk;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    auto chunk = optional<Chunk>();
    try {
      chunk = std::move(result).get();
    } catch(const std::exception&) {}
    if(chunk) {
      on_read(generation, target, std::move(chunk->m_entries), chunk->m_bbo,
        chunk->m_anchor);
    } else {
      on_read(generation, target, none, none, none);
    }
  });
}

void ReplayTimeAndSalesModel::seek() {
  ++m_read_generation;
  m_is_reading = false;
  m_is_seeking = true;
  m_buffer.clear();
  m_bbo = none;
  stop_live();
  m_read_end = std::min(m_read_end, m_position);
  m_last_read_time = ptime(neg_infin);
  m_anchor = none;
  request_seek();
}

void ReplayTimeAndSalesModel::request_seek() {
  if(m_is_seek_querying || m_seek_timer.isActive()) {
    return;
  }
  query_seek();
}

void ReplayTimeAndSalesModel::query_seek() {
  m_seek_timer.start();
  if(!m_entries.empty() &&
      m_entries.front().m_time_and_sale->m_timestamp <= m_position) {
    auto size = get_size();
    m_transaction.transact([&] {
      while(m_entries.back().m_time_and_sale->m_timestamp > m_position) {
        pop_back();
      }
    });
    if(get_size() < size && get_size() >= m_page_size) {
      m_is_seeking = false;
      m_read_end = m_position;
      read();
      return;
    }
  }
  m_is_seek_querying = true;
  auto generation = m_read_generation;
  auto position = m_position;
  m_seek_promise = QtPromise([=, client = m_client, ticker = m_ticker,
      range = Range(get_start(), position), count = m_page_size] {
    auto chunk = read_chunk(
      client, ticker, range, SnapshotLimit::from_tail(count), none);
    if(chunk.m_entries.empty()) {
      chunk.m_anchor = load_anchor(client, ticker, position);
    }
    return chunk;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    auto chunk = optional<Chunk>();
    try {
      chunk = std::move(result).get();
    } catch(const std::exception&) {}
    if(chunk) {
      on_seek(generation, position, std::move(chunk->m_entries), chunk->m_bbo,
        chunk->m_anchor);
    } else {
      on_seek(generation, position, none, none, none);
    }
  });
}

void ReplayTimeAndSalesModel::reconcile() {
  if(m_is_seeking) {
    return;
  }
  if(m_session->get_state()->get() == PlaybackState::REAL_TIME) {
    read();
  } else {
    stop_live();
    m_transaction.transact([&] {
      while(!m_entries.empty() &&
          m_entries.back().m_time_and_sale->m_timestamp > m_position) {
        auto entry = m_entries.back();
        pop_back();
        m_buffer.push_front(std::move(entry));
      }
    });
    publish();
    read();
  }
}

void ReplayTimeAndSalesModel::start_live() {
  if(m_live_handler) {
    return;
  }
  ++m_read_generation;
  m_is_reading = false;
  m_transaction.transact([&] {
    while(!m_buffer.empty()) {
      append(std::move(m_buffer.front()));
      m_buffer.pop_front();
    }
  });
  m_live_handler = std::make_unique<EventHandler>();
  auto bbo_query = make_real_time_query(m_ticker);
  bbo_query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(bbo_query, m_live_handler->get_slot<SequencedBboQuote>(
    std::bind_front(&ReplayTimeAndSalesModel::on_bbo, this)));
  auto query = TickerQuery();
  query.set_index(m_ticker);
  query.set_range(get_cursor(), Beam::Sequence::LAST);
  query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(query, m_live_handler->get_slot<SequencedTimeAndSale>(
    std::bind_front(&ReplayTimeAndSalesModel::on_time_and_sale, this)));
}

void ReplayTimeAndSalesModel::stop_live() {
  m_live_handler = nullptr;
}

void ReplayTimeAndSalesModel::on_read(int generation, ptime end,
    optional<std::vector<Type>> entries, optional<SequencedBboQuote> bbo,
    optional<Beam::Sequence> anchor) {
  if(generation != m_read_generation) {
    return;
  }
  m_is_reading = false;
  if(!entries) {
    return;
  }
  if(anchor) {
    m_anchor = anchor;
  }
  m_bbo = std::move(bbo);
  if(std::ssize(*entries) == READ_SIZE) {
    m_read_end =
      entries->back().m_time_and_sale->m_timestamp - microseconds(1);
    m_last_read_time = ptime(neg_infin);
  } else {
    m_read_end = std::max(m_read_end, end);
  }
  for(auto& entry : *entries) {
    auto sequence = entry.m_time_and_sale.get_sequence();
    if(!m_buffer.empty() &&
        sequence <= m_buffer.back().m_time_and_sale.get_sequence()) {
      continue;
    }
    m_buffer.push_back(std::move(entry));
  }
  publish();
  if(m_session->get_state()->get() == PlaybackState::REAL_TIME &&
      std::ssize(*entries) != READ_SIZE) {
    start_live();
  } else {
    read();
  }
}

void ReplayTimeAndSalesModel::on_seek(int generation, ptime position,
    optional<std::vector<Type>> entries, optional<SequencedBboQuote> bbo,
    optional<Beam::Sequence> anchor) {
  m_is_seek_querying = false;
  if(generation != m_read_generation) {
    if(m_is_seeking) {
      request_seek();
    }
    return;
  }
  m_is_seeking = false;
  if(entries) {
    if(!merge(*entries)) {
      replace(std::move(*entries));
    }
    m_bbo = std::move(bbo);
    m_anchor = anchor;
    m_read_end = position;
  } else {
    replace({});
    m_read_end = position;
  }
  reconcile();
}

void ReplayTimeAndSalesModel::on_seek_timeout() {
  if(m_is_seeking && !m_is_seek_querying) {
    query_seek();
  }
}

void ReplayTimeAndSalesModel::on_older(
    int generation, std::vector<Type> entries) {
  if(generation != m_older_generation) {
    return;
  }
  auto begin = std::find_if(entries.begin(), entries.end(),
    [&] (const auto& entry) {
      return entry.m_time_and_sale->m_timestamp >= get_start();
    });
  auto last = std::upper_bound(begin, entries.end(), m_position,
    [] (const auto& position, const auto& entry) {
      return position < entry.m_time_and_sale->m_timestamp;
    });
  auto end = [&] {
    if(m_entries.empty()) {
      return last;
    }
    return std::lower_bound(begin, last,
      m_entries.front().m_time_and_sale.get_sequence(),
      [] (const auto& entry, const auto& bound) {
        return entry.m_time_and_sale.get_sequence() < bound;
      });
  }();
  if(begin == end) {
    return;
  }
  m_transaction.transact([&] {
    for(auto i = end; i != begin; --i) {
      m_entries.push_front(std::move(*std::prev(i)));
      m_transaction.push(AddOperation(0));
    }
  });
}

void ReplayTimeAndSalesModel::on_bbo(const SequencedBboQuote& bbo) {
  m_bbo = bbo;
}

void ReplayTimeAndSalesModel::on_time_and_sale(
    const SequencedTimeAndSale& time_and_sale) {
  auto bbo = [&] {
    if(m_bbo) {
      return **m_bbo;
    }
    return BboQuote();
  }();
  append(Type(time_and_sale, get_bbo_indicator(bbo, *time_and_sale)));
}

void ReplayTimeAndSalesModel::on_timeline(const Timeline& timeline) {
  auto cursor = get_cursor();
  while(!m_buffer.empty() &&
      m_buffer.front().m_time_and_sale->m_timestamp < timeline.m_start) {
    m_buffer.pop_front();
  }
  m_transaction.transact([&] {
    while(!m_entries.empty() &&
        m_entries.front().m_time_and_sale->m_timestamp < timeline.m_start) {
      pop_front();
    }
  });
  if(m_buffer.empty() && m_entries.empty()) {
    if(auto sequence = std::get_if<Beam::Sequence>(&cursor)) {
      m_anchor = *sequence;
    }
  }
}

void ReplayTimeAndSalesModel::on_position(const ptime& position) {
  auto previous = m_position;
  m_position = position;
  auto is_real_time =
    m_session->get_state()->get() == PlaybackState::REAL_TIME;
  if(position < previous ||
      (!is_real_time && position - previous > get_jump_threshold())) {
    seek();
  } else {
    reconcile();
  }
}

void ReplayTimeAndSalesModel::on_speed(const Decimal& speed) {
  read();
}

void ReplayTimeAndSalesModel::on_state(PlaybackState state) {
  reconcile();
}
