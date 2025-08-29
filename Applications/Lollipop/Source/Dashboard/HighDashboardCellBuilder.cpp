#include "Spire/Dashboard/HighDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

std::unique_ptr<DashboardCell> HighDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& serviceClients = userProfile.Get()->GetClients();
  auto query = make_daily_high_query(security,
    serviceClients.get_time_client().GetTime(), pos_infin,
    userProfile.Get()->GetVenueDatabase(),
    userProfile.Get()->GetTimeZoneDatabase());
  auto baseQueue = std::make_shared<Queue<Nexus::QueryVariant>>();
  std::shared_ptr<QueueReader<Money>> queue =
    MakeConverterQueueReader(baseQueue,
      [] (const Nexus::QueryVariant& value) {
        return boost::get<Money>(value);
      });
  serviceClients.get_charting_client().query(query, baseQueue);
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  return cell;
}

std::unique_ptr<DashboardCellBuilder> HighDashboardCellBuilder::Clone() const {
  return std::make_unique<HighDashboardCellBuilder>();
}
