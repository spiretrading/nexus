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
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;
using namespace Nexus::TechnicalAnalysis;
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
  auto selfUserProfile = userProfile.Get();
  Spawn(
    [=] {
      auto& serviceClients = selfUserProfile->GetServiceClients();
      auto close = LoadPreviousClose(serviceClients.GetMarketDataClient(),
        security, serviceClients.GetTimeClient().GetTime(),
        selfUserProfile->GetMarketDatabase(),
        selfUserProfile->GetTimeZoneDatabase());
      if(!close.is_initialized()) {
        baseQueue->Break();
        return;
      }
      *closePrice = close->m_price;
      auto& marketDataClient = serviceClients.GetMarketDataClient();
      auto marketStartOfDay = MarketDateToUtc(security.GetMarket(),
        serviceClients.GetTimeClient().GetTime(),
        selfUserProfile->GetMarketDatabase(),
        selfUserProfile->GetTimeZoneDatabase());
      auto query = MakeCurrentQuery(security);
      marketDataClient.QueryTimeAndSales(query, baseQueue);
    });
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder>
    ChangeDashboardCellBuilder::Clone() const {
  return std::make_unique<ChangeDashboardCellBuilder>();
}
