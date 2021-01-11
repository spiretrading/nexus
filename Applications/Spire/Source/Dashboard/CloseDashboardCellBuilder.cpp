#include "Spire/Dashboard/CloseDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
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

std::unique_ptr<DashboardCell> CloseDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto queue = std::make_shared<Queue<DashboardCell::Value>>();
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  auto selfUserProfile = userProfile.Get();
  Spawn(
    [=] {
      auto& serviceClients = selfUserProfile->GetServiceClients();
      auto close = LoadPreviousClose(serviceClients.GetMarketDataClient(),
        security, serviceClients.GetTimeClient().GetTime(),
        selfUserProfile->GetMarketDatabase(),
        selfUserProfile->GetTimeZoneDatabase());
      if(close.is_initialized()) {
        queue->Push(close->m_price);
      }
      queue->Break();
    });
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder> CloseDashboardCellBuilder::Clone() const {
  return std::make_unique<CloseDashboardCellBuilder>();
}
