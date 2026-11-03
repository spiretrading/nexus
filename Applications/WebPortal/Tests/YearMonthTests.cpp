#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "WebPortal/ReportFile.hpp"
#include "WebPortal/ReportSubmission.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("YearMonth") {
  TEST_CASE("report_parameters") {
    auto yaml = YAML::Load(R"(
id: monthly
name: Monthly
access: ['*']
parameters:
  - {name: month, label: Month, type: YearMonth, required: true,
      default: 2026-02-01}
command: echo
arguments: ['{month}']
output: {media_type: text/plain, extension: txt, filename: 'monthly_{month}'}
)");
    auto definition = parse_report_definition(yaml);
    REQUIRE(definition.m_parameters.front().m_type == "YearMonth");
    REQUIRE(*definition.m_parameters.front().m_default == "2026-02-01");
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto values = prepare_report_parameters(
      definition, JsonObject(), client.get_account(), client);
    REQUIRE(values.at("month") == "20260201");
    REQUIRE(make_report_arguments(definition, values) ==
      std::vector<std::string>({"20260201"}));
    REQUIRE(format_report_parameter(definition.m_parameters.front(),
      values.at("month")) == "2026-02");
    auto input = JsonObject();
    for(auto& text : {"2025-12-31", "20251231", "20251201"}) {
      input.set("month", text);
      values = prepare_report_parameters(
        definition, input, client.get_account(), client);
      REQUIRE(values.at("month") == to_iso_string(date(2025, 12, 1)));
    }
    auto job = ReportJob();
    job.m_definition = definition;
    job.m_parameters = values;
    job.m_created = time_from_string("2026-11-02 12:00:00");
    REQUIRE(make_report_filename(job) == "monthly_20251201.txt");
    for(auto invalid : {JsonValue("2026-02"), JsonValue("20260229"),
        JsonValue("2026-13-01"), JsonValue("1399-12-01"),
        JsonValue("2026-02-01\n"), JsonValue(20260201),
        JsonValue(JsonObject())}) {
      input["month"] = invalid;
      REQUIRE_THROWS_AS(prepare_report_parameters(
        definition, input, client.get_account(), client), std::invalid_argument);
    }
    definition.m_parameters.front().m_default.reset();
    REQUIRE_THROWS_AS(prepare_report_parameters(
      definition, JsonObject(), client.get_account(), client),
      std::invalid_argument);
    definition.m_parameters.front().m_is_required = false;
    REQUIRE(!prepare_report_parameters(definition, JsonObject(),
      client.get_account(), client).get("month"));
    yaml["parameters"][0]["default"] = "2026-13-01";
    REQUIRE_THROWS_AS(prepare_report_parameters(parse_report_definition(yaml),
      JsonObject(), client.get_account(), client), std::invalid_argument);
  }
}
