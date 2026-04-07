#include "Spire/Dashboard/LastPriceDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterQueueReader.hpp>
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

std::unique_ptr<DashboardCell> LastPriceDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& marketDataClient =
    userProfile.get()->GetClients().get_market_data_client();
  auto baseQueue = std::make_shared<Queue<TimeAndSale>>();
  auto queue = std::static_pointer_cast<QueueReader<Money>>(
    convert(baseQueue, [] (const TimeAndSale& timeAndSale) {
      return timeAndSale.m_price;
    }));
  auto query = make_current_query(security);
  marketDataClient.query(query, baseQueue);
  return std::make_unique<QueueDashboardCell>(queue);
}

std::unique_ptr<DashboardCellBuilder>
    LastPriceDashboardCellBuilder::Clone() const {
  return std::make_unique<LastPriceDashboardCellBuilder>();
}
