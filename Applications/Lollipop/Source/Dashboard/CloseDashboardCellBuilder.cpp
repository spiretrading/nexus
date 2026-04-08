#include "Spire/Dashboard/CloseDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardTickerQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

std::unique_ptr<DashboardCell> CloseDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& ticker = boost::get<Ticker>(index);
  auto queue = std::make_shared<Queue<DashboardCell::Value>>();
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  auto selfUserProfile = userProfile.get();
  spawn([=] {
    auto& serviceClients = selfUserProfile->GetClients();
    auto close = load_previous_close(serviceClients.get_market_data_client(),
      ticker, serviceClients.get_time_client().get_time(),
      selfUserProfile->GetVenueDatabase(),
      selfUserProfile->GetTimeZoneDatabase());
    if(close) {
      queue->push(close->m_price);
    }
    queue->close();
  });
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder> CloseDashboardCellBuilder::Clone() const {
  return std::make_unique<CloseDashboardCellBuilder>();
}
