#include "Spire/TimeAndSales/ServicesTimeAndSalesModel.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
#include "Spire/Spire/Definitions.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::TechnicalAnalysis;
using namespace Spire;

ServicesTimeAndSalesModel::ServicesTimeAndSalesModel(Security security,
    const Definitions& definitions, ServiceClientsBox clients)
    : m_security(std::move(security)),
      m_clients(std::move(clients)) {
  auto query = BuildRealTimeQuery(m_security);
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_clients.GetMarketDataClient().QueryBboQuotes(query,
    m_event_handler.get_slot<SequencedBboQuote>(
      [=] (const auto& bbo) { on_bbo(bbo); }));
  m_clients.GetMarketDataClient().QueryTimeAndSales(query,
    m_event_handler.get_slot<SequencedTimeAndSale>(
      [=] (const auto& time_and_sale) { on_time_and_sale(time_and_sale); }));
  QueryDailyVolume(m_clients.GetChartingClient(), m_security,
    m_clients.GetTimeClient().GetTime(), pos_infin,
    definitions.get_market_database(), definitions.get_time_zone_database(),
    m_event_handler.get_slot<Quantity>(
      [=] (const auto& volume) { m_volume_signal(volume); }));
}

const Security& ServicesTimeAndSalesModel::get_security() const {
  return m_security;
}

Quantity ServicesTimeAndSalesModel::get_volume() const {
  return m_volume;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    ServicesTimeAndSalesModel::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  return QtPromise(
    [last, count, security = m_security, clients = m_clients] () mutable {
      auto query = SecurityMarketDataQuery();
      query.SetIndex(security);
      query.SetRange(Beam::Queries::Sequence::First(), last);
      query.SetSnapshotLimit(SnapshotLimit::FromTail(count));
      auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
      clients.GetMarketDataClient().QueryTimeAndSales(query, queue);
      auto result = std::vector<TimeAndSalesModel::Entry>();
      try {
        while(true) {
          auto time_and_sale = queue->Pop();
          result.push_back(Entry{std::move(time_and_sale),
            TimeAndSalesProperties::PriceRange::UNKNOWN});
        }
      } catch(const PipeBrokenException&) {
      }
      return result;
    }, LaunchPolicy::ASYNC);
}

connection ServicesTimeAndSalesModel::connect_time_and_sale_signal(
    const TimeAndSaleSignal::slot_type& slot) const {
  return m_time_and_sale_signal.connect(slot);
}

connection ServicesTimeAndSalesModel::connect_volume_signal(
    const VolumeSignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}

void ServicesTimeAndSalesModel::on_bbo(const SequencedBboQuote& bbo) {
  m_bbo = bbo;
}

void ServicesTimeAndSalesModel::on_time_and_sale(
    const SequencedTimeAndSale& time_and_sale) {
  auto range = [&] {
    if(m_bbo->m_ask.m_price == Money::ZERO) {
      return TimeAndSalesProperties::PriceRange::UNKNOWN;
    } else if(time_and_sale->m_price == m_bbo->m_bid.m_price) {
      return TimeAndSalesProperties::PriceRange::AT_BID;
    } else if(time_and_sale->m_price < m_bbo->m_bid.m_price) {
      return TimeAndSalesProperties::PriceRange::BELOW_BID;
    } else if(time_and_sale->m_price == m_bbo->m_ask.m_price) {
      return TimeAndSalesProperties::PriceRange::AT_ASK;
    } else if(time_and_sale->m_price > m_bbo->m_ask.m_price) {
      return TimeAndSalesProperties::PriceRange::ABOVE_ASK;
    }
    return TimeAndSalesProperties::PriceRange::INSIDE;
  }();
  m_time_and_sale_signal({time_and_sale, range});
}
