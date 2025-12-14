#include "Spire/Charting/SecurityTimePriceChartPlotSeries.hpp"
#include "Spire/Charting/CandlestickChartPlot.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto BASE_DATE = ptime(date(1970, Jan, 1), seconds(0));

  time_duration Normalize(const ptime& time, time_duration interval) {
    auto totalTicks = time - BASE_DATE;
    return microseconds(
      totalTicks.total_microseconds() % interval.total_microseconds());
  }
}

std::size_t SecurityTimePriceChartPlotSeries::TimestampHash::operator ()(
    ptime value) const {
  auto totalTicks = value - BASE_DATE;
  return static_cast<std::size_t>(totalTicks.ticks());
}

SecurityTimePriceChartPlotSeries::SecurityTimePriceChartPlotSeries(
    Ref<UserProfile> userProfile, const Security& security,
    time_duration interval)
    : m_userProfile(userProfile.get()),
      m_security(security),
      m_interval(interval) {
  if(security == Security()) {
    return;
  }
  auto query = SecurityMarketDataQuery();
  query.set_index(security);
  query.set_range(Beam::Range::REAL_TIME);
  query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetClients().get_market_data_client().query(
    query, m_eventHandler.get_slot<TimeAndSale>(
      std::bind_front(&SecurityTimePriceChartPlotSeries::OnTimeAndSale, this)));
}

void SecurityTimePriceChartPlotSeries::Query(ChartValue start, ChartValue end) {
  m_taskQueue.push(
    [=] {
      auto min = start.ToDateTime() - Normalize(start.ToDateTime(), m_interval);
      auto max = end.ToDateTime() +
        (m_interval - Normalize(end.ToDateTime(), m_interval));
      auto result = m_userProfile->GetClients().get_charting_client().
        load_time_price_series(m_security, min, max, m_interval);
      for(auto& candlestick : result.series) {
        auto chartCandlestick = Candlestick(
          ChartValue(candlestick.get_start()),
          ChartValue(candlestick.get_end()),
          ChartValue(candlestick.get_open()),
          ChartValue(candlestick.get_close()),
          ChartValue(candlestick.get_high()),
          ChartValue(candlestick.get_low()));
        m_eventHandler.push(std::bind_front(
          &SecurityTimePriceChartPlotSeries::OnCandlestickLoaded, this,
          chartCandlestick));
      }
    });
}

ChartValue SecurityTimePriceChartPlotSeries::LoadLastCurrentDomain() {
  auto timeAndSalesQueue = std::make_shared<Queue<TimeAndSale>>();
  auto query = SecurityMarketDataQuery();
  query.set_index(m_security);
  query.set_range(Beam::Sequence::FIRST, Beam::Sequence::PRESENT);
  query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetClients().get_market_data_client().query(
    query, timeAndSalesQueue);
  try {
    return ChartValue(timeAndSalesQueue->pop().m_timestamp);
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(std::runtime_error("Unable to load last point."));
  }
}

connection SecurityTimePriceChartPlotSeries::ConnectChartPointAddedSignal(
    const ChartPlotAddedSignal::slot_function_type& slot) const {
  return m_chartPlotAddedSignal.connect(slot);
}

SecurityTimePriceChartPlotSeries::CandlestickEntry&
    SecurityTimePriceChartPlotSeries::LoadCandlestick(ptime timestamp) {
  auto key = timestamp - Normalize(timestamp, m_interval);
  auto entryIterator = m_candlestickEntries.find(key);
  if(entryIterator == m_candlestickEntries.end()) {
    entryIterator =
      m_candlestickEntries.insert(std::pair(key, CandlestickEntry())).first;
    auto& entry = entryIterator->second;
    entry.m_plot = std::make_shared<CandlestickChartPlot>(
      ChartValue(key), ChartValue(key + m_interval));
  }
  return entryIterator->second;
}

void SecurityTimePriceChartPlotSeries::OnCandlestickLoaded(
    const Candlestick<ChartValue, ChartValue>& candlestick) {
  auto& entry = LoadCandlestick(candlestick.get_start().ToDateTime());
  auto isFirstUpdate = entry.m_lastTimestamp.is_not_a_date_time();
  if(!isFirstUpdate && entry.m_lastTimestamp >=
      candlestick.get_end().ToDateTime()) {
    return;
  }
  entry.m_lastTimestamp = candlestick.get_start().ToDateTime();
  entry.m_plot->Update(candlestick.get_open());
  entry.m_plot->Update(candlestick.get_high());
  entry.m_plot->Update(candlestick.get_low());
  entry.m_plot->Update(candlestick.get_close());
  if(isFirstUpdate) {
    m_chartPlotAddedSignal(entry.m_plot);
  }
}

void SecurityTimePriceChartPlotSeries::OnTimeAndSale(
    const TimeAndSale& timeAndSale) {
  auto& entry = LoadCandlestick(timeAndSale.m_timestamp);
  auto isFirstUpdate = entry.m_lastTimestamp.is_not_a_date_time();
  if(!isFirstUpdate && entry.m_lastTimestamp >= timeAndSale.m_timestamp) {
    return;
  }
  entry.m_lastTimestamp = timeAndSale.m_timestamp;
  entry.m_plot->Update(ChartValue(timeAndSale.m_price));
  if(isFirstUpdate) {
    m_chartPlotAddedSignal(entry.m_plot);
  }
}
