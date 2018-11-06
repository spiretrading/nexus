#include "spire/time_and_sales/services_time_and_sales_model.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServicesTimeAndSalesModel::ServicesTimeAndSalesModel(Security security,
    Ref<VirtualMarketDataClient> client)
    : m_security(std::move(security)),
      m_client(client.Get()) {}

const Security& ServicesTimeAndSalesModel::get_security() const {
  return m_security;
}

Quantity ServicesTimeAndSalesModel::get_volume() const {
  return 0;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    ServicesTimeAndSalesModel::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  return make_qt_promise([last, count, security=m_security, client = m_client] {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Beam::Queries::Sequence::First(), last);
    query.SetSnapshotLimit(SnapshotLimit::FromTail(count));
    auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
    client->QueryTimeAndSales(query, queue);
    auto result = std::vector<TimeAndSalesModel::Entry>();
    try {
      while(true) {
        auto time_and_sale = queue->Top();
        queue->Pop();
        result.push_back(Entry{std::move(time_and_sale),
          TimeAndSalesProperties::PriceRange::UNKNOWN});
      }
    } catch(const PipeBrokenException&) {
    }
    return result;
  });
}

connection ServicesTimeAndSalesModel::connect_time_and_sale_signal(
    const TimeAndSaleSignal::slot_type& slot) const {
  return {};
}

connection ServicesTimeAndSalesModel::connect_volume_signal(
    const VolumeSignal::slot_type& slot) const {
  return {};
}
