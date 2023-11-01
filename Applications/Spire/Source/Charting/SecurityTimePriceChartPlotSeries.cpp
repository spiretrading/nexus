#include "Spire/Charting/SecurityTimePriceChartPlotSeries.hpp"
#include "Spire/Charting/CandlestickChartPlot.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::TechnicalAnalysis;
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
    : m_userProfile(userProfile.Get()),
      m_security(security),
      m_interval(interval) {
  if(security == Security()) {
    return;
  }
  auto query = SecurityMarketDataQuery();
  query.SetIndex(security);
  query.SetRange(Beam::Queries::Range::RealTime());
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    query, m_eventHandler.get_slot<TimeAndSale>(
      std::bind_front(&SecurityTimePriceChartPlotSeries::OnTimeAndSale, this)));
}

void SecurityTimePriceChartPlotSeries::Query(ChartValue start, ChartValue end) {
  m_taskQueue.Push(
    [=] {
      auto min = start.ToDateTime() - Normalize(start.ToDateTime(), m_interval);
      auto max = end.ToDateTime() +
        (m_interval - Normalize(end.ToDateTime(), m_interval));
      auto result = m_userProfile->GetServiceClients().GetChartingClient().
        LoadTimePriceSeries(m_security, min, max, m_interval);
      for(auto& candlestick : result.series) {
        auto chartCandlestick = Candlestick(
          ChartValue(candlestick.GetStart()), ChartValue(candlestick.GetEnd()),
          ChartValue(candlestick.GetOpen()), ChartValue(candlestick.GetClose()),
          ChartValue(candlestick.GetHigh()), ChartValue(candlestick.GetLow()));
        m_eventHandler.push(std::bind_front(
          &SecurityTimePriceChartPlotSeries::OnCandlestickLoaded, this,
          chartCandlestick));
      }
    });
}

ChartValue SecurityTimePriceChartPlotSeries::LoadLastCurrentDomain() {
  auto timeAndSalesQueue = std::make_shared<Queue<TimeAndSale>>();
  auto query = SecurityMarketDataQuery();
  query.SetIndex(m_security);
  query.SetRange(
    Beam::Queries::Sequence::First(), Beam::Queries::Sequence::Present());
  query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1);
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    query, timeAndSalesQueue);
  try {
    return ChartValue(timeAndSalesQueue->Pop().m_timestamp);
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
  auto& entry = LoadCandlestick(candlestick.GetStart().ToDateTime());
  auto isFirstUpdate = entry.m_lastTimestamp.is_not_a_date_time();
  if(!isFirstUpdate && entry.m_lastTimestamp >=
      candlestick.GetEnd().ToDateTime()) {
    return;
  }
  entry.m_lastTimestamp = candlestick.GetStart().ToDateTime();
  entry.m_plot->Update(candlestick.GetOpen());
  entry.m_plot->Update(candlestick.GetHigh());
  entry.m_plot->Update(candlestick.GetLow());
  entry.m_plot->Update(candlestick.GetClose());
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
