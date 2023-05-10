#include <doctest/doctest.h>
#include "boost/date_time/c_local_time_adjustor.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <locale>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

auto to_string(ptime t) {
  auto ss = std::ostringstream();
  auto facet = new time_facet();
  facet->format("%Y-%m-%d %H:%M:%S%F");
  ss.imbue(std::locale(std::locale::classic(), facet));
  ss << t;
  return ss.str();
}

TEST_SUITE("ExportTable") {
  TEST_CASE("export_as_csv") {
    auto date1 = date(2023, Jan, 20);
    auto time_duration1 = time_duration(1, 2, 3, 741874);
    auto date2 = date(2023, Mar, 2);
    auto time_duration2 = time_duration(3, 4, 5, 645650);
    auto date3 = date(2023, May, 11);
    auto time_duration3 = time_duration(5, 6, 7);
    auto time1 = ptime(date1, time_duration1);
    auto time2 = ptime(date2, time_duration2);
    auto time3 = ptime(date3, time_duration3);
    auto local_time1 = date_time::c_local_adjustor<ptime>::utc_to_local(time1);
    auto local_time2 = date_time::c_local_adjustor<ptime>::utc_to_local(time2);
    auto local_time3 = date_time::c_local_adjustor<ptime>::utc_to_local(time3);
    auto result = std::format(
      "\"Time\",\"Date\",\"Duration\",\"Int\",\"Double\",\"Price\","
      "\"Quantity\",\"Market\",\"Security\",\"Country\",\"Condition\"\n"
      R"({},{},{},100,1234.56,1000.55,1000.32,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},{},{},1000,0.21,20.00,10.5,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},{},{},10000,1000,30.12,4000,"NYSE","MRU.TSX","CAN","@")",
      to_string(local_time1), to_iso_extended_string(date1),
      to_simple_string(time_duration1),
      to_string(local_time2), to_iso_extended_string(date2),
      to_simple_string(time_duration2),
      to_string(local_time3), to_iso_extended_string(date3),
      to_simple_string(time_duration3));
    auto table = ArrayTableModel();
    auto market = MarketToken(MarketCode("XNYS"));
    auto security = ParseWildCardSecurity(
      "MRU.TSX", GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    auto country = DefaultCountries::CA();
    auto condition =
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@");
    table.push({"Time", "Date", "Duration", "Int", "Double", "Price",
      "Quantity", "Market", "Security", "Country", "Condition"});
    table.push({time1, date1, time_duration1, 100, 1234.56, Money(1000.55),
      Quantity(1000.32), market, *security, country, condition});
    table.push({time2, date2, time_duration2, 1000, 0.21, Money(20),
      Quantity(10.5), market, *security, country, condition});
    table.push({time3, date3, time_duration3, 10000, 1000, Money(30.12),
      Quantity(4000), market, *security, country, condition});
    auto out = std::stringstream();
    export_table_as_csv(table, out);
    REQUIRE(out.str() == result);
  }
}
