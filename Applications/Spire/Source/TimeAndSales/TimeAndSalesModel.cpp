#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto BBO_PAGE_SIZE = 1000;

  template<typename T>
  std::vector<T> load(MarketDataClient& client, const Ticker& ticker,
      const Range& range, const SnapshotLimit& limit) {
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(range);
    query.set_snapshot_limit(limit);
    auto queue = std::make_shared<Queue<T>>();
    client.query(query, queue);
    auto result = std::vector<T>();
    try {
      while(true) {
        result.push_back(queue->pop());
      }
    } catch(const PipeBrokenException&) {}
    return result;
  }
}

std::vector<SequencedTimeAndSale> Spire::load_time_and_sales(
    MarketDataClient& client, const Ticker& ticker, const Range& range,
    const SnapshotLimit& limit) {
  return load<SequencedTimeAndSale>(client, ticker, range, limit);
}

BboIndicatorsLoadResult Spire::load_bbo_indicators(
    MarketDataClient& client, const Ticker& ticker,
    const std::vector<SequencedTimeAndSale>& time_and_sales,
    optional<SequencedBboQuote> initial_bbo) {
  auto indicators = std::vector<BboIndicator>();
  auto bbo = std::move(initial_bbo);
  if(time_and_sales.empty()) {
    return BboIndicatorsLoadResult(std::move(indicators), std::move(bbo));
  }
  auto first = time_and_sales.front()->m_timestamp;
  auto last = time_and_sales.back()->m_timestamp;
  if(!bbo || (*bbo)->m_timestamp > first) {
    bbo = none;
    auto head = load<SequencedBboQuote>(client, ticker,
      Range(Beam::Sequence::FIRST, first), SnapshotLimit::from_tail(1));
    if(!head.empty()) {
      bbo = head.back();
    }
  }
  auto get_start = [&] () -> Range::Point {
    if(bbo) {
      return increment(bbo->get_sequence());
    }
    return first;
  };
  auto quotes = load<SequencedBboQuote>(client, ticker,
    Range(get_start(), last), SnapshotLimit::from_head(BBO_PAGE_SIZE));
  auto index = std::size_t(0);
  for(auto& time_and_sale : time_and_sales) {
    auto& timestamp = time_and_sale->m_timestamp;
    while(true) {
      while(index < quotes.size() &&
          quotes[index]->m_timestamp <= timestamp) {
        bbo = quotes[index];
        ++index;
      }
      if(index < quotes.size() || std::ssize(quotes) < BBO_PAGE_SIZE) {
        break;
      }
      auto skip = load<SequencedBboQuote>(client, ticker,
        Range(get_start(), timestamp), SnapshotLimit::from_tail(1));
      if(!skip.empty()) {
        bbo = skip.back();
      }
      quotes = load<SequencedBboQuote>(client, ticker,
        Range(get_start(), last), SnapshotLimit::from_head(BBO_PAGE_SIZE));
      index = 0;
    }
    auto current = [&] {
      if(bbo) {
        return **bbo;
      }
      return BboQuote();
    }();
    indicators.push_back(get_bbo_indicator(current, *time_and_sale));
  }
  return BboIndicatorsLoadResult(std::move(indicators), std::move(bbo));
}
