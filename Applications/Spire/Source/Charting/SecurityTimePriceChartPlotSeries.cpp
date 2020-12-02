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
using namespace std;

namespace {
  static const int UPDATE_INTERVAL = 500;

  time_duration Normalize(const ptime& time, time_duration interval) {
    static ptime BASE_DATE(date(1970, Jan, 1), seconds(0));
    time_duration totalTicks = time - BASE_DATE;
    return microseconds(
      totalTicks.total_microseconds() % interval.total_microseconds());
  }
}

size_t SecurityTimePriceChartPlotSeries::TimestampHash::operator ()(
    const ptime& value) const {
  static ptime BASE_DATE(date(1970, Jan, 1), seconds(0));
  time_duration totalTicks = value - BASE_DATE;
  return static_cast<size_t>(totalTicks.ticks());
}

SecurityTimePriceChartPlotSeries::SecurityTimePriceChartPlotSeries(
    Ref<UserProfile> userProfile, const Security& security,
    const time_duration& interval)
    : m_userProfile(userProfile.Get()),
      m_security(security),
      m_interval(interval),
      m_updateTimer(new QTimer()) {
  if(security == Security()) {
    return;
  }
  SecurityMarketDataQuery query;
  query.SetIndex(security);
  query.SetRange(Beam::Queries::Range::RealTime());
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    query, m_slotHandler.GetSlot<TimeAndSale>(
    std::bind(&SecurityTimePriceChartPlotSeries::OnTimeAndSale, this,
    std::placeholders::_1)));
  std::function<void ()> timeoutSlot = std::bind(
    &SecurityTimePriceChartPlotSeries::OnUpdateTimer, this);
  m_timerConnection = QObject::connect(m_updateTimer, &QTimer::timeout,
    timeoutSlot);
  m_updateTimer->start(UPDATE_INTERVAL);
}

SecurityTimePriceChartPlotSeries::~SecurityTimePriceChartPlotSeries() {
  QObject::disconnect(m_timerConnection);
  m_updateTimer->deleteLater();
}

void SecurityTimePriceChartPlotSeries::Query(ChartValue start, ChartValue end) {
  m_taskQueue.Push(
    [=] {
      auto min = start.ToDateTime() - Normalize(start.ToDateTime(), m_interval);
      auto max = end.ToDateTime() +
        (m_interval - Normalize(end.ToDateTime(), m_interval));
      auto result = m_userProfile->GetServiceClients().GetChartingClient().
        LoadTimePriceSeries(m_security, min, max, m_interval);
      for(const TimePriceCandlestick& candlestick : result.series) {
        Candlestick<ChartValue, ChartValue> chartCandlestick(
          ChartValue(candlestick.GetStart()), ChartValue(candlestick.GetEnd()),
          ChartValue(candlestick.GetOpen()), ChartValue(candlestick.GetClose()),
          ChartValue(candlestick.GetHigh()), ChartValue(candlestick.GetLow()));
        m_slotHandler.Push(
          std::bind(&SecurityTimePriceChartPlotSeries::OnCandlestickLoaded,
          this, chartCandlestick));
      }
    });
}

ChartValue SecurityTimePriceChartPlotSeries::LoadLastCurrentDomain() {
  std::shared_ptr<Queue<TimeAndSale>> timeAndSalesQueue =
    std::make_shared<Queue<TimeAndSale>>();
  SecurityMarketDataQuery query;
  query.SetIndex(m_security);
  query.SetRange(Beam::Queries::Sequence::First(),
    Beam::Queries::Sequence::Present());
  query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1);
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    query, timeAndSalesQueue);
  ChartValue domain;
  try {
    domain = ChartValue(timeAndSalesQueue->Pop().m_timestamp);
  } catch(const std::exception&) {
    BOOST_THROW_EXCEPTION(std::runtime_error("Unable to load last point."));
  }
  return domain;
}

connection SecurityTimePriceChartPlotSeries::ConnectChartPointAddedSignal(
    const ChartPlotAddedSignal::slot_function_type& slot) const {
  return m_chartPlotAddedSignal.connect(slot);
}

SecurityTimePriceChartPlotSeries::CandlestickEntry&
    SecurityTimePriceChartPlotSeries::LoadCandlestick(const ptime& timestamp) {
  static ptime BASE_DATE(date(1970, Jan, 1), seconds(0));
  ptime key = timestamp - Normalize(timestamp, m_interval);
  auto entryIterator = m_candlestickEntries.find(key);
  if(entryIterator == m_candlestickEntries.end()) {
    entryIterator = m_candlestickEntries.insert(
      std::make_pair(key, CandlestickEntry())).first;
    CandlestickEntry& entry = entryIterator->second;
    entry.m_plot = std::make_shared<CandlestickChartPlot>(ChartValue(key),
      ChartValue(key + m_interval));
  }
  return entryIterator->second;
}

void SecurityTimePriceChartPlotSeries::OnCandlestickLoaded(
    const Candlestick<ChartValue, ChartValue>& candlestick) {
  CandlestickEntry& entry = LoadCandlestick(
    candlestick.GetStart().ToDateTime());
  bool isFirstUpdate = entry.m_lastTimestamp.is_not_a_date_time();
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
  CandlestickEntry& entry = LoadCandlestick(timeAndSale.m_timestamp);
  bool isFirstUpdate =
    entry.m_lastTimestamp.is_not_a_date_time();
  if(!isFirstUpdate && entry.m_lastTimestamp >= timeAndSale.m_timestamp) {
    return;
  }
  entry.m_lastTimestamp = timeAndSale.m_timestamp;
  entry.m_plot->Update(ChartValue(timeAndSale.m_price));
  if(isFirstUpdate) {
    m_chartPlotAddedSignal(entry.m_plot);
  }
}

void SecurityTimePriceChartPlotSeries::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
