#include "Nexus/Python/TechnicalAnalysis.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"
#include "Nexus/TechnicalAnalysis/StandardTickerQueries.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_technical_analysis(module& module) {
  export_candlestick<Candlestick<object, object>>(module, "Candlestick");
  export_candlestick<PriceCandlestick>(module, "PriceCandlestick");
  export_standard_ticker_queries(module);
}

void Nexus::Python::export_standard_ticker_queries(module& module) {
  module.def("make_open_query", &make_open_query);
  module.def("load_open",
    [] (MarketDataClient& client, const Ticker& ticker, ptime date,
        const VenueDatabase& venues, const tz_database& time_zones) {
      return load_open(client, ticker, date, venues, time_zones);
    }, call_guard<GilRelease>());
  module.def("query_open",
    [] (SharedObject shared_client, const Ticker& ticker, ptime date,
        const VenueDatabase& venues, const tz_database& time_zones,
        ScopedQueueWriter<TimeAndSale> queue) {
      auto& client = shared_client->cast<MarketDataClient&>();
      return spawn([=, &client, shared_client = std::move(shared_client),
          queue = std::move(queue)] mutable {
        auto query = RoutineHandler(query_open(client, ticker, date, venues,
          time_zones, std::move(queue)));
        query.wait();
      });
    });
  module.def("make_previous_close_query", &make_previous_close_query);
  module.def("load_previous_close",
    [] (MarketDataClient& client, const Ticker& ticker, ptime date,
        const VenueDatabase& venues, const tz_database& time_zones) {
      return load_previous_close(client, ticker, date, venues, time_zones);
    }, call_guard<GilRelease>());
  module.def("make_daily_query_range", &make_daily_query_range);
  module.def("make_query", &make_query);
  module.def("make_daily_high_query", &make_daily_high_query);
  module.def("query_daily_high",
    [] (ChartingClient& client, const Ticker& ticker, ptime start, ptime end,
        const VenueDatabase& venues, const tz_database& time_zones,
        ScopedQueueWriter<Money> queue) {
      query_daily_high(client, ticker, start, end, venues, time_zones,
        std::move(queue));
    });
  module.def("make_daily_low_query", &make_daily_low_query);
  module.def("query_daily_low",
    [] (ChartingClient& client, const Ticker& ticker, ptime start, ptime end,
        const VenueDatabase& venues, const tz_database& time_zones,
        ScopedQueueWriter<Money> queue) {
      query_daily_low(client, ticker, start, end, venues, time_zones,
        std::move(queue));
    });
  module.def("make_daily_volume_query", &make_daily_volume_query);
  module.def("query_daily_volume",
    [] (ChartingClient& client, Ticker ticker, ptime start, ptime end,
        const VenueDatabase& venues, const tz_database& time_zones,
        ScopedQueueWriter<Quantity> queue) {
      query_daily_volume(client, ticker, start, end, venues, time_zones,
        std::move(queue));
    });
}
