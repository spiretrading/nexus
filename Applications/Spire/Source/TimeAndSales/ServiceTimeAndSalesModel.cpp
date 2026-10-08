#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  BboIndicator get_indicator(
      const BboQuote& bbo, const TimeAndSale& time_and_sale) {
    if(bbo.m_ask.m_price == Money::ZERO) {
      return BboIndicator::UNKNOWN;
    } else if(time_and_sale.m_price == bbo.m_bid.m_price) {
      return BboIndicator::AT_BID;
    } else if(time_and_sale.m_price < bbo.m_bid.m_price) {
      return BboIndicator::BELOW_BID;
    } else if(time_and_sale.m_price == bbo.m_ask.m_price) {
      return BboIndicator::AT_ASK;
    } else if(time_and_sale.m_price > bbo.m_ask.m_price) {
      return BboIndicator::ABOVE_ASK;
    }
    return BboIndicator::INSIDE;
  }

  std::vector<SequencedBboQuote> load_bbo_quotes(MarketDataClient& client,
      const Ticker& ticker, ptime first_timestamp, ptime last_timestamp) {
    auto result = std::vector<SequencedBboQuote>();
    auto head_query = TickerQuery();
    head_query.set_index(ticker);
    head_query.set_range(Beam::Sequence::FIRST, first_timestamp);
    head_query.set_snapshot_limit(SnapshotLimit::from_tail(1));
    auto head_queue = std::make_shared<Queue<SequencedBboQuote>>();
    client.query(head_query, head_queue);
    try {
      result.push_back(head_queue->pop());
    } catch(const PipeBrokenException&) {}
    auto tail_query = TickerQuery();
    tail_query.set_index(ticker);
    if(result.empty()) {
      tail_query.set_range(first_timestamp, last_timestamp);
    } else {
      tail_query.set_range(
        increment(result.back().get_sequence()), last_timestamp);
    }
    tail_query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto tail_queue = std::make_shared<Queue<SequencedBboQuote>>();
    client.query(tail_query, tail_queue);
    try {
      while(true) {
        result.push_back(tail_queue->pop());
      }
    } catch(const PipeBrokenException&) {}
    return result;
  }

  void replay_bbo_indicators(MarketDataClient& client, const Ticker& ticker,
      std::vector<Spire::Details::TimeAndSalesEntry>& entries) {
    if(entries.empty()) {
      return;
    }
    auto bbos = load_bbo_quotes(client, ticker,
      entries.front().m_time_and_sale->m_timestamp,
      entries.back().m_time_and_sale->m_timestamp);
    auto bbo = BboQuote();
    auto index = std::size_t(0);
    for(auto& entry : entries) {
      auto& timestamp = entry.m_time_and_sale->m_timestamp;
      while(index < bbos.size() && bbos[index]->m_timestamp <= timestamp) {
        bbo = *bbos[index];
        ++index;
      }
      entry.m_indicator = get_indicator(bbo, *entry.m_time_and_sale);
    }
  }
}

ServiceTimeAndSalesModel::ServiceTimeAndSalesModel(
    Ticker ticker, MarketDataClient client)
    : m_ticker(std::move(ticker)),
      m_client(std::move(client)) {
  auto query = make_real_time_query(m_ticker);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(query, m_event_handler.get_slot<SequencedBboQuote>(
    std::bind_front(&ServiceTimeAndSalesModel::on_bbo, this)));
  m_client.query(query, m_event_handler.get_slot<SequencedTimeAndSale>(
    std::bind_front(&ServiceTimeAndSalesModel::on_time_and_sale, this)));
}

QtPromise<void> ServiceTimeAndSalesModel::load_older(int max_count) {
  auto sequence = [&] {
    if(m_entries.get_size() == 0) {
      return Beam::Sequence::PRESENT;
    }
    return m_entries.get(0).m_time_and_sale.get_sequence();
  }();
  return QtPromise([=, ticker = m_ticker, client = m_client] () mutable {
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Beam::Sequence::FIRST, sequence);
    query.set_snapshot_limit(SnapshotLimit::from_tail(max_count));
    auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
    client.query(query, queue);
    auto result = std::vector<Spire::Details::TimeAndSalesEntry>();
    try {
      while(true) {
        auto time_and_sale = queue->pop();
        result.push_back(Spire::Details::TimeAndSalesEntry(
          std::move(time_and_sale), BboIndicator::UNKNOWN));
      }
    } catch(const PipeBrokenException&) {}
    replay_bbo_indicators(client, ticker, result);
    return result;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    try {
      auto& snapshot = result.get();
      auto end = [&] {
        if(m_entries.get_size() == 0) {
          return snapshot.end();
        }
        return std::lower_bound(snapshot.begin(), snapshot.end(),
          m_entries.get(0).m_time_and_sale.get_sequence(),
          [] (const auto& entry, const auto& bound) {
            return entry.m_time_and_sale.get_sequence() < bound;
          });
      }();
      auto count = static_cast<int>(std::distance(snapshot.begin(), end));
      if(count != 0) {
        m_entries.transact([&] {
          for(auto i = 0; i < count; ++i) {
            m_entries.insert(snapshot[i], i);
          }
        });
      }
    } catch(const std::exception&) {}
  });
}

int ServiceTimeAndSalesModel::get_size() const {
  return m_entries.get_size();
}

const ServiceTimeAndSalesModel::Type&
    ServiceTimeAndSalesModel::get(int index) const {
  return m_entries.get(index);
}

connection ServiceTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_entries.connect_operation_signal(slot);
}

void ServiceTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  m_entries.transact([&] {
    transaction();
  });
}

void ServiceTimeAndSalesModel::on_bbo(const SequencedBboQuote& bbo) {
  m_bbo = bbo;
}

void ServiceTimeAndSalesModel::on_time_and_sale(
    const SequencedTimeAndSale& time_and_sale) {
  if(m_entries.get_size() != 0 && time_and_sale.get_sequence() <=
      m_entries.get(m_entries.get_size() - 1).m_time_and_sale.get_sequence()) {
    return;
  }
  m_entries.push(Spire::Details::TimeAndSalesEntry(
    time_and_sale, get_indicator(*m_bbo, *time_and_sale)));
}
