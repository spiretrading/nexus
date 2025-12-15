#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

ServiceTimeAndSalesModel::ServiceTimeAndSalesModel(
    Security security, MarketDataClient client)
    : m_security(std::move(security)),
      m_client(std::move(client)) {
  auto query = make_real_time_query(m_security);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_client.query(query, m_event_handler.get_slot<SequencedBboQuote>(
    std::bind_front(&ServiceTimeAndSalesModel::on_bbo, this)));
  m_client.query(query, m_event_handler.get_slot<SequencedTimeAndSale>(
    std::bind_front(&ServiceTimeAndSalesModel::on_time_and_sale, this)));
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    ServiceTimeAndSalesModel::query_until(
      Beam::Sequence sequence, int max_count) {
  return QtPromise(
    [sequence, max_count, security = m_security, client = m_client] () mutable {
      auto query = SecurityMarketDataQuery();
      query.set_index(security);
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
  auto range = [&] {
    if(m_bbo->m_ask.m_price == Money::ZERO) {
      return BboIndicator::UNKNOWN;
    } else if(time_and_sale->m_price == m_bbo->m_bid.m_price) {
      return BboIndicator::AT_BID;
    } else if(time_and_sale->m_price < m_bbo->m_bid.m_price) {
      return BboIndicator::BELOW_BID;
    } else if(time_and_sale->m_price == m_bbo->m_ask.m_price) {
      return BboIndicator::AT_ASK;
    } else if(time_and_sale->m_price > m_bbo->m_ask.m_price) {
      return BboIndicator::ABOVE_ASK;
    }
    return BboIndicator::INSIDE;
  }();
  m_update_signal({time_and_sale, range});
}
