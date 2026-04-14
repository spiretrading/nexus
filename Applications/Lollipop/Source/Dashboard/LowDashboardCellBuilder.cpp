#include "Spire/Dashboard/LowDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardTickerQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

std::unique_ptr<DashboardCell> LowDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& ticker = boost::get<Ticker>(index);
  auto& serviceClients = userProfile.get()->GetClients();
  auto query = make_daily_low_query(ticker,
    serviceClients.get_time_client().get_time(), pos_infin,
    userProfile.get()->GetVenueDatabase(),
    userProfile.get()->GetTimeZoneDatabase());
  auto baseQueue = std::make_shared<Queue<Nexus::QueryVariant>>();
  auto queue = std::static_pointer_cast<QueueReader<Money>>(
    convert(baseQueue, [] (const Nexus::QueryVariant& value) {
      return boost::get<Money>(value);
    }));
  serviceClients.get_charting_client().query(query, baseQueue);
  return std::make_unique<QueueDashboardCell>(queue);
}

std::unique_ptr<DashboardCellBuilder> LowDashboardCellBuilder::Clone() const {
  return std::make_unique<LowDashboardCellBuilder>();
}
