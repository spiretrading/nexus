#include <doctest/doctest.h>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("ExportTable") {
  TEST_CASE("export_as_csv") {
    auto ts1 = std::string("01:02:03");
    auto ts2 = std::string("03:04:05");
    auto ts3 = std::string("05:06:07");
    auto result = std::format(
      R"("Time","Price","Quantity","Market","Security","Country","Condition")"
      "\n"
      R"({},10.00,1,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},20.00,2,"NYSE","MRU.TSX","CAN","@")""\n"
      R"({},30.00,4,"NYSE","MRU.TSX","CAN","@")",
      ts1, ts2, ts3);
    auto table = ArrayTableModel();
    auto market = MarketToken(MarketCode("XNYS"));
    auto security = ParseWildCardSecurity(
      "MRU.TSX", GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    auto country = DefaultCountries::CA();
    auto condition =
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@");
    table.push({"Time", "Price", "Quantity", "Market", "Security", "Country",
      "Condition"});
    table.push({duration_from_string(ts1), Money(10), Quantity(1), market,
      *security, country, condition});
    table.push({duration_from_string(ts2), Money(20), Quantity(2), market,
      *security, country, condition});
    table.push({duration_from_string(ts3), Money(30), Quantity(4), market,
      *security, country, condition});
    auto out = std::stringstream();
    export_table_as_csv(table, out);
    REQUIRE(out.str() == result);
  }
}
