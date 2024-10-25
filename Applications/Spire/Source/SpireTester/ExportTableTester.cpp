#include <doctest/doctest.h>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::TimeService;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("ExportTable") {
  TEST_CASE("export_as_csv") {
    auto ts1 = std::string("2023-01-20 01:02:03.741874");
    auto ts2 = std::string("2023-03-02 03:04:05.645650");
    auto ts3 = std::string("2023-05-11 05:06:07");
    auto local_time1 = time_from_string(ts1);
    auto local_time2 = time_from_string(ts2);
    auto local_time3 = time_from_string(ts3);
    auto time1 = ToUtcTime(local_time1);
    auto time2 = ToUtcTime(local_time2);
    auto time3 = ToUtcTime(local_time3);
    auto date1 = local_time1.date();
    auto date2 = local_time2.date();
    auto date3 = local_time3.date();
    auto time_duration1 = local_time1.time_of_day();
    auto time_duration2 = local_time2.time_of_day();
    auto time_duration3 = local_time3.time_of_day();
    auto result = std::format(
      R"({},{},{},100,1234.56,1000.55,1000.32,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},{},{},1000,0.21,20.00,10.5,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},{},{},10000,1000,30.12,4000,"NYSE","MRU.TSX","CAN","@")",
      ts1, ts1.substr(0, 10), ts1.substr(11),
      ts2, ts2.substr(0, 10), ts2.substr(11),
      ts3, ts3.substr(0, 10), ts3.substr(11));
    auto table = ArrayTableModel();
    auto market = MarketToken(MarketCode("XNYS"));
    auto security = ParseSecurity("MRU.TSX");
    auto country = DefaultCountries::CA();
    auto condition =
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@");
    table.push({time1, date1, time_duration1, 100, 1234.56, Money(1000.55),
      Quantity(1000.32), market, security, country, condition});
    table.push({time2, date2, time_duration2, 1000, 0.21, Money(20),
      Quantity(10.5), market, security, country, condition});
    table.push({time3, date3, time_duration3, 10000, 1000, Money(30.12),
      Quantity(4000), market, security, country, condition});
    auto out = std::stringstream();
    export_table_as_csv(table, {}, out);
    REQUIRE(out.str() == result);
    result = std::format(
      "\"Time\",\"Date\",\"Duration\",\"Int\",\"Double\",\"Price\","
      "\"Quantity\",\"Market\",\"Security\",\"Country\",\"Condition\"\n") +
      result;
    out.str("");
    auto headers = std::vector<QString>{
      "Time", "Date", "Duration", "Int", "Double", "Price",
      "Quantity", "Market", "Security", "Country", "Condition"};
    export_table_as_csv(table, headers, out);
    REQUIRE(out.str() == result);
  }
}
