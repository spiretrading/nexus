#include "Spire/Dashboard/BboSizeDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterQueueReader.hpp>
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

BboSizeDashboardCellBuilder::BboSizeDashboardCellBuilder(Side side)
  : m_side{side} {}

std::unique_ptr<DashboardCell> BboSizeDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& marketDataClient =
    userProfile.get()->GetClients().get_market_data_client();
  auto baseQueue = std::make_shared<Queue<BboQuote>>();
  auto side = m_side;
  auto queue = std::static_pointer_cast<QueueReader<Quantity>>(
    convert(baseQueue, [=] (const BboQuote& quote) {
      return pick(side, quote.m_ask.m_size, quote.m_bid.m_size);
    }));
  auto query = make_current_query(security);
  marketDataClient.query(query, baseQueue);
  return std::make_unique<QueueDashboardCell>(queue);
}

std::unique_ptr<DashboardCellBuilder> BboSizeDashboardCellBuilder::
    Clone() const {
  return std::make_unique<BboSizeDashboardCellBuilder>(m_side);
}
