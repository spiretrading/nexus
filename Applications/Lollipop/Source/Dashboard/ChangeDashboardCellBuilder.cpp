#include "Spire/Dashboard/ChangeDashboardCellBuilder.hpp"
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
using namespace Spire;
using namespace std;

std::unique_ptr<DashboardCell> ChangeDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  std::shared_ptr<Money> closePrice = std::make_shared<Money>();
  auto baseQueue = std::make_shared<Queue<TimeAndSale>>();
  std::shared_ptr<QueueReader<double>> queue =
    MakeConverterQueueReader(baseQueue,
      [=] (const TimeAndSale& timeAndSale) {
        return (timeAndSale.m_price - *closePrice) / *closePrice;
      });
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  auto selfUserProfile = userProfile.get();
  Spawn(
    [=] {
      auto& serviceClients = selfUserProfile->GetClients();
      auto close = load_previous_close(serviceClients.get_market_data_client(),
        security, serviceClients.get_time_client().GetTime(),
        selfUserProfile->GetVenueDatabase(),
        selfUserProfile->GetTimeZoneDatabase());
      if(!close.is_initialized()) {
        baseQueue->Break();
        return;
      }
      *closePrice = close->m_price;
      auto& marketDataClient = serviceClients.get_market_data_client();
      auto venueStartOfDay = venue_date_to_utc(security.get_venue(),
        serviceClients.get_time_client().GetTime(),
        selfUserProfile->GetVenueDatabase(),
        selfUserProfile->GetTimeZoneDatabase());
      auto query = MakeCurrentQuery(security);
      marketDataClient.query(query, baseQueue);
    });
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder>
    ChangeDashboardCellBuilder::Clone() const {
  return std::make_unique<ChangeDashboardCellBuilder>();
}
