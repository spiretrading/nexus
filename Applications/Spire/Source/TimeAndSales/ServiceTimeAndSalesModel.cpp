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
      std::vector<TimeAndSalesModel::Entry>& entries) {
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

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    ServiceTimeAndSalesModel::query_until(
      Beam::Sequence sequence, int max_count) {
  return QtPromise([=, ticker = m_ticker, client = m_client] () mutable {
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Beam::Sequence::FIRST, sequence);
    query.set_snapshot_limit(SnapshotLimit::from_tail(max_count));
    auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
    client.query(query, queue);
    auto result = std::vector<TimeAndSalesModel::Entry>();
    try {
      while(true) {
        auto time_and_sale = queue->pop();
        result.push_back(
          Entry(std::move(time_and_sale), BboIndicator::UNKNOWN));
      }
    } catch(const PipeBrokenException&) {}
    replay_bbo_indicators(client, ticker, result);
    return result;
  }, LaunchPolicy::ASYNC);
}

connection ServiceTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void ServiceTimeAndSalesModel::on_bbo(const SequencedBboQuote& bbo) {
  m_bbo = bbo;
}

void ServiceTimeAndSalesModel::on_time_and_sale(
    const SequencedTimeAndSale& time_and_sale) {
  m_update_signal({time_and_sale, get_indicator(*m_bbo, *time_and_sale)});
}
