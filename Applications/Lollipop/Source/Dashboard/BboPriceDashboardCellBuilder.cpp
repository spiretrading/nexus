#include "Spire/Dashboard/BboPriceDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterQueueReader.hpp>
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

BboPriceDashboardCellBuilder::BboPriceDashboardCellBuilder(Side side)
    : m_side{side} {}

std::unique_ptr<DashboardCell> BboPriceDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& marketDataClient =
    userProfile.Get()->GetServiceClients().GetMarketDataClient();
  auto baseQueue = std::make_shared<Queue<BboQuote>>();
  auto side = m_side;
  std::shared_ptr<QueueReader<Money>> queue =
    MakeConverterQueueReader(baseQueue,
      [=] (const BboQuote& quote) {
        return Pick(side, quote.m_ask.m_price, quote.m_bid.m_price);
      });
  auto query = MakeCurrentQuery(security);
  marketDataClient.QueryBboQuotes(query, baseQueue);
  auto last = std::make_unique<QueueDashboardCell>(queue);
  return std::move(last);
}

std::unique_ptr<DashboardCellBuilder> BboPriceDashboardCellBuilder::
    Clone() const {
  return std::make_unique<BboPriceDashboardCellBuilder>(m_side);
}
