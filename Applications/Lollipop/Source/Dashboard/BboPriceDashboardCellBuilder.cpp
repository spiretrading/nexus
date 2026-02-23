#include "Spire/Dashboard/BboPriceDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterQueueReader.hpp>
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

BboPriceDashboardCellBuilder::BboPriceDashboardCellBuilder(Side side)
    : m_side{side} {}

std::unique_ptr<DashboardCell> BboPriceDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& ticker = boost::get<Ticker>(index);
  auto& marketDataClient =
    userProfile.get()->GetClients().get_market_data_client();
  auto baseQueue = std::make_shared<Queue<BboQuote>>();
  auto side = m_side;
  auto queue = std::static_pointer_cast<QueueReader<Money>>(
    convert(baseQueue, [=] (const BboQuote& quote) {
      return pick(side, quote.m_ask.m_price, quote.m_bid.m_price);
    }));
  auto query = make_current_query(ticker);
  marketDataClient.query(query, baseQueue);
  return std::make_unique<QueueDashboardCell>(queue);
}

std::unique_ptr<DashboardCellBuilder> BboPriceDashboardCellBuilder::
    Clone() const {
  return std::make_unique<BboPriceDashboardCellBuilder>(m_side);
}
