#include "Spire/Dashboard/ChangeDashboardCellBuilder.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
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
  auto queue = std::static_pointer_cast<QueueReader<double>>(
    convert(baseQueue, [=] (const TimeAndSale& timeAndSale) {
      return (timeAndSale.m_price - *closePrice) / *closePrice;
    }));
  auto cell = std::make_unique<QueueDashboardCell>(queue);
  auto selfUserProfile = userProfile.get();
  spawn([=] {
    auto& serviceClients = selfUserProfile->GetClients();
    auto close = load_previous_close(serviceClients.get_market_data_client(),
      security, serviceClients.get_time_client().get_time(),
      selfUserProfile->GetVenueDatabase(),
      selfUserProfile->GetTimeZoneDatabase());
    if(!close) {
      baseQueue->close();
      return;
    }
    *closePrice = close->m_price;
    auto& marketDataClient = serviceClients.get_market_data_client();
    auto venueStartOfDay = venue_date_to_utc(security.get_venue(),
      serviceClients.get_time_client().get_time(),
      selfUserProfile->GetVenueDatabase(),
      selfUserProfile->GetTimeZoneDatabase());
    auto query = make_current_query(security);
    marketDataClient.query(query, baseQueue);
  });
  return cell;
}

std::unique_ptr<DashboardCellBuilder>
    ChangeDashboardCellBuilder::Clone() const {
  return std::make_unique<ChangeDashboardCellBuilder>();
}
