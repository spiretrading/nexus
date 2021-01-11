#include "Nexus/Python/TechnicalAnalysis.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ChartingService/ChartingClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace Nexus::TechnicalAnalysis;
using namespace pybind11;

namespace {
  auto candlestick =
    std::unique_ptr<class_<Candlestick<object, object>>>();
}

class_<Candlestick<object, object>>& Nexus::Python::GetExportedCandlestick() {
  return *candlestick;
}

void Nexus::Python::ExportTechnicalAnalysis(module& module) {
  auto submodule = module.def_submodule("technical_analysis");
  candlestick = std::make_unique<class_<Candlestick<object, object>>>(
    ExportCandlestick<Candlestick<object, object>>(submodule, "Candlestick"));
  ExportCandlestick<TimePriceCandlestick>(submodule, "TimePriceCandlestick");
  submodule.def("get_default_market_center", &GetDefaultMarketCenter);
  submodule.def("make_open_query", static_cast<
    SecurityMarketDataQuery (*)(Security, ptime, const MarketDatabase&,
      const tz_database&, std::string)>(&MakeOpenQuery));
  submodule.def("make_open_query", static_cast<
    SecurityMarketDataQuery (*)(Security, ptime, const MarketDatabase&,
      const tz_database&)>(&MakeOpenQuery));
  submodule.def("load_open", static_cast<
    optional<TimeAndSale> (*)(MarketDataClientBox&, Security, ptime,
      const MarketDatabase&, const tz_database&, std::string)>(
        &LoadOpen<MarketDataClientBox>), call_guard<GilRelease>());
  submodule.def("load_open", static_cast<
    optional<TimeAndSale> (*)(MarketDataClientBox&, Security, ptime,
      const MarketDatabase&, const tz_database&)>(
        &LoadOpen<MarketDataClientBox>), call_guard<GilRelease>());
  submodule.def("query_open",
    [] (MarketDataClientBox client, Security security, ptime date,
        const MarketDatabase& marketDatabase,
        const tz_database& timeZoneDatabase, std::string marketCenter,
        ScopedQueueWriter<TimeAndSale> queue) {
      QueryOpen(std::move(client), std::move(security), date, marketDatabase,
        timeZoneDatabase, std::move(marketCenter), std::move(queue));
    });
  submodule.def("query_open",
    [] (MarketDataClientBox client, Security security, ptime date,
        const MarketDatabase& marketDatabase,
        const tz_database& timeZoneDatabase,
        ScopedQueueWriter<TimeAndSale> queue) {
      QueryOpen(std::move(client), std::move(security), date, marketDatabase,
        timeZoneDatabase, std::move(queue));
    });
  submodule.def("make_previous_close_query", static_cast<
    SecurityMarketDataQuery (*)(Security, ptime, const MarketDatabase&,
      const tz_database&, std::string)>(&MakePreviousCloseQuery));
  submodule.def("make_previous_close_query", static_cast<
    SecurityMarketDataQuery (*)(Security, ptime, const MarketDatabase&,
      const tz_database&)>(&MakePreviousCloseQuery));
  submodule.def("load_previous_close", static_cast<
    optional<TimeAndSale> (*)(MarketDataClientBox&, Security, ptime,
      const MarketDatabase&, const tz_database&, std::string)>(
        &LoadPreviousClose<MarketDataClientBox>), call_guard<GilRelease>());
  submodule.def("load_previous_close", static_cast<
    optional<TimeAndSale> (*)(MarketDataClientBox&, Security, ptime,
      const MarketDatabase&, const tz_database&)>(
        &LoadPreviousClose<MarketDataClientBox>), call_guard<GilRelease>());
  submodule.def("make_daily_high_query", &MakeDailyHighQuery);
  submodule.def("query_daily_high", &QueryDailyHigh<ChartingClientBox>);
  submodule.def("make_daily_low_query", &MakeDailyLowQuery);
  submodule.def("query_daily_low", &QueryDailyLow<ChartingClientBox>);
  submodule.def("make_daily_volume_query", &MakeDailyVolumeQuery);
  submodule.def("query_daily_volume", &QueryDailyVolume<ChartingClientBox>);
}
