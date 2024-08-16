#include "Spire/Dashboard/VolumeDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;
using namespace Nexus::TechnicalAnalysis;
using namespace Spire;
using namespace std;

std::unique_ptr<DashboardCell> VolumeDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& serviceClients = userProfile.Get()->GetServiceClients();
  auto query = MakeDailyVolumeQuery(security,
    serviceClients.GetTimeClient().GetTime(), pos_infin,
    userProfile.Get()->GetMarketDatabase(),
    userProfile.Get()->GetTimeZoneDatabase());
  auto baseQueue = std::make_shared<Queue<Nexus::Queries::QueryVariant>>();
  std::shared_ptr<QueueReader<Quantity>> queue =
    MakeConverterQueueReader(baseQueue,
      [] (const Nexus::Queries::QueryVariant& value) {
        return boost::get<Quantity>(value);
      });
  serviceClients.GetChartingClient().QuerySecurity(query, baseQueue);
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder>
    VolumeDashboardCellBuilder::Clone() const {
  return std::make_unique<VolumeDashboardCellBuilder>();
}
