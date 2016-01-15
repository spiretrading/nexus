#include "Spire/Dashboard/BboSizeDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterReaderQueue.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

BboSizeDashboardCellBuilder::BboSizeDashboardCellBuilder(Side side)
    : m_side{side} {}

std::unique_ptr<DashboardCell> BboSizeDashboardCellBuilder::Build(
    const DashboardCell::Value& index, RefType<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& marketDataClient =
    userProfile.Get()->GetServiceClients().GetMarketDataClient();
  auto baseQueue = std::make_shared<Queue<BboQuote>>();
  auto side = m_side;
  std::shared_ptr<QueueReader<Quantity>> queue =
    MakeConverterReaderQueue<Quantity>(baseQueue,
    [=] (const BboQuote& quote) {
      return Pick(side, quote.m_ask.m_size, quote.m_bid.m_size);
    });
  auto query = QueryRealTimeWithSnapshot(security);
  marketDataClient.QueryBboQuotes(query, baseQueue);
  auto last = std::make_unique<QueueDashboardCell>(queue);
  return std::move(last);
}

std::unique_ptr<DashboardCellBuilder> BboSizeDashboardCellBuilder::
    Clone() const {
  return std::make_unique<BboSizeDashboardCellBuilder>(m_side);
}
