#include <sstream>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;

namespace {
  int execute(const ReportJob&, std::stop_token) {
    return 0;
  }

  struct DefinitionService {
    int m_loads = 0;

    void update_schedule(const DirectoryEntry&, const std::string&,
        const ReportScheduleSubmission&) {
      throw std::runtime_error("Unexpected schedule update.");
    }

    ReportSchedule load_schedule(const DirectoryEntry&, const std::string&) {
      throw std::runtime_error("Unexpected schedule lookup.");
    }

    ScheduledReports query(const DirectoryEntry&, const ScheduledReportQuery&) {
      throw std::runtime_error("Unexpected schedule query.");
    }

    void store(const ReportSchedule&) {
      throw std::runtime_error("Unexpected schedule storage.");
    }

    GeneratedReports query(const DirectoryEntry&, const GeneratedReportQuery&) {
      throw std::runtime_error("Unexpected generated report query.");
    }

    ReportDetail load_report(const DirectoryEntry&, const std::string&) {
      throw std::runtime_error("Unexpected report detail query.");
    }

    ReportFile load_file(const DirectoryEntry&, const std::string&) {
      throw std::runtime_error("Unexpected file query.");
    }

    ReportActivities query(const DirectoryEntry&, const ReportActivityQuery&) {
      throw std::runtime_error("Unexpected activity query.");
    }

    std::vector<ReportDefinition> load_definitions(const DirectoryEntry&) {
      ++m_loads;
      return {};
    }

    std::string submit(const DirectoryEntry&, const ReportSubmission&) {
      throw std::runtime_error("Unexpected report submission.");
    }

    std::string submit(const DirectoryEntry&, const ReportScheduleSubmission&) {
      throw std::runtime_error("Unexpected report submission.");
    }

    void remove_schedule(const DirectoryEntry&, const std::string&) {
      throw std::runtime_error("Unexpected schedule deletion.");
    }

    void share(const DirectoryEntry&, const std::vector<std::string>&,
        const std::vector<DirectoryEntry>&) {
      throw std::runtime_error("Unexpected report sharing.");
    }

    void remove(const DirectoryEntry&, const std::vector<std::string>&) {
      throw std::runtime_error("Unexpected report deletion.");
    }

    void remove(const std::string&) {
      throw std::runtime_error("Unexpected job removal.");
    }

    void cancel(const DirectoryEntry&, const std::vector<std::string>&) {
      throw std::runtime_error("Unexpected report cancellation.");
    }

    void retry(const DirectoryEntry&, const std::vector<std::string>&) {
      throw std::runtime_error("Unexpected report retry.");
    }

    std::optional<ReportJob> load_job(const std::string&) {
      throw std::runtime_error("Unexpected job lookup.");
    }

    void store(const ReportJob&) {
      throw std::runtime_error("Unexpected job storage.");
    }

    int execute(const ReportJob&, std::stop_token) {
      throw std::runtime_error("Unexpected job execution.");
    }

    void close() {}
  };

  YAML::Node make_definition() {
    return YAML::Load(R"(
id: profit_and_loss
name: Profit and Loss
description: Generate a profit and loss report.
access: [Reporting]
parameters:
  - name: account
    label: Account / Group
    type: DirectoryEntry
    required: true
  - name: period
    label: Date Range
    type: DateRange
  - name: currency
    label: Currency
    type: Currency
    required: true
    default: USD
command: /opt/spire/reports/profit_and_loss
arguments:
  - --account
  - "{account.id}"
  - if_present: period.start
    arguments: [--start, "{period.start}"]
output:
  media_type: text/csv
  extension: csv
)");
  }

  ReportDefinition make_definition(
      std::string id, std::vector<std::string> access) {
    auto definition = parse_report_definition(make_definition());
    definition.m_id = id;
    definition.m_access = access;
    return definition;
  }

  HttpRequest make_request(const WebPortalSession& session) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/load_report_definitions"),
      from<SharedBuffer>("{}"));
    request.add(Cookie("sessionid", session.get_id()));
    return request;
  }

  HttpResponse load(ReportingWebServlet& servlet, const HttpRequest& request) {
    auto slots = servlet.get_slots();
    auto slot = std::ranges::find_if(slots,
      [&] (const auto& slot) { return slot.m_predicate(request); });
    REQUIRE(slot != slots.end());
    return slot->m_slot(request);
  }

  std::vector<JsonValue> parse_response(const HttpResponse& response) {
    auto value = parse<JsonValue>(response.get_body());
    return get<std::vector<JsonValue>>(value);
  }
}

TEST_SUITE("ReportDefinition") {
  TEST_CASE("load_definition") {
    auto node = make_definition();
    auto source = std::istringstream(YAML::Dump(node));
    auto definition = load_report_definition(source);
    REQUIRE(definition.m_id == "profit_and_loss");
    REQUIRE(definition.m_command == "/opt/spire/reports/profit_and_loss");
    REQUIRE(definition.m_access == std::vector<std::string>({"Reporting"}));
    REQUIRE(to_json(definition) == to_json(parse_report_definition(node)));
    auto invalid = std::istringstream("id: [invalid");
    REQUIRE_THROWS_AS(load_report_definition(invalid), YAML::ParserException);
  }

  TEST_CASE("parse_definition") {
    auto definition = parse_report_definition(make_definition());
    REQUIRE(definition.m_id == "profit_and_loss");
    REQUIRE(definition.m_access == std::vector<std::string>({"Reporting"}));
    REQUIRE(definition.m_parameters.size() == 3);
    REQUIRE(definition.m_parameters[0].m_is_required);
    REQUIRE_FALSE(definition.m_parameters[1].m_is_required);
    REQUIRE(!definition.m_parameters[1].m_default);
    REQUIRE(get<std::string>(*definition.m_parameters[2].m_default) == "USD");
    REQUIRE(definition.m_arguments.size() == 3);
    auto& arguments =
      std::get<OptionalReportArguments>(definition.m_arguments[2]);
    REQUIRE(arguments.m_if_present == "period.start");
    REQUIRE(arguments.m_arguments ==
      std::vector<std::string>({"--start", "{period.start}"}));
    auto encoded = parse<JsonValue>(to_json(definition));
    auto& object = get<JsonObject>(encoded);
    REQUIRE(get<std::vector<JsonValue>>(object.at("access"))[0] == "Reporting");
    REQUIRE(object.at("command") == "/opt/spire/reports/profit_and_loss");
    REQUIRE(
      get<JsonObject>(object.at("output")).at("media_type") == "text/csv");
    auto& parameters = get<std::vector<JsonValue>>(object.at("parameters"));
    REQUIRE(!get<JsonObject>(parameters[0]).get("default"));
    REQUIRE(get<JsonObject>(parameters[2]).at("default") == "USD");
  }

  TEST_CASE("parameter_defaults") {
    auto node = make_definition();
    node["description"] = "Line one\n\"Line two\"\\file";
    node["parameters"] = YAML::Load(R"(
- name: integer
  label: Integer
  type: Integer
  default: 0
- name: decimal
  label: Decimal
  type: Decimal
  default: 0.000000125
- name: text
  label: Money
  type: Money
  default: "001"
- name: date
  label: Date
  type: Date
  default: null
- name: range
  label: Date Range
  type: DateRange
  default: {start: "2026-10-01", end: "2026-10-04"}
- name: entries
  label: Accounts
  type: DirectoryEntryList
  default: [{type: 0, id: 42, name: "001"}]
- name: optional
  label: Optional
  type: Integer
)");
    auto definition = parse_report_definition(node);
    auto decoded = parse<JsonValue>(to_json(definition));
    auto& decoded_object = get<JsonObject>(decoded);
    REQUIRE(decoded_object.at("description") == "Line one\n\"Line two\"\\file");
    auto& parameters =
      get<std::vector<JsonValue>>(decoded_object.at("parameters"));
    REQUIRE(get<double>(get<JsonObject>(parameters[0]).at("default")) == 0);
    REQUIRE(
      get<double>(get<JsonObject>(parameters[1]).at("default")) == 0.000000125);
    REQUIRE(get<JsonObject>(parameters[2]).at("default") == "001");
    auto& date = get<JsonObject>(parameters[3]);
    REQUIRE(static_cast<bool>(date.get("default")));
    REQUIRE(get<JsonNull>(&date.at("default")));
    auto& range = get<JsonObject>(get<JsonObject>(parameters[4]).at("default"));
    REQUIRE(range.at("start") == "2026-10-01");
    auto& entries =
      get<std::vector<JsonValue>>(get<JsonObject>(parameters[5]).at("default"));
    REQUIRE(get<JsonObject>(entries[0]).at("id") == 42);
    REQUIRE(get<JsonObject>(entries[0]).at("name") == "001");
    test_round_trip_shuttle(definition, [&] (const auto& received) {
      REQUIRE(parse<JsonValue>(to_json(received)) ==
        parse<JsonValue>(to_json(definition)));
    });
    auto buffer = from<SharedBuffer>(to_json(definition));
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = ReportDefinition();
    receiver.shuttle(received);
    REQUIRE(parse<JsonValue>(to_json(received)) ==
      parse<JsonValue>(to_json(definition)));
  }

  TEST_CASE("invalid_definitions") {
    auto node = make_definition();
    node["parameters"].push_back(node["parameters"][0]);
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["access"] = "Reporting";
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["arguments"][2]["arguments"] = "--start";
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["arguments"][0] = "  ";
    node["arguments"][2]["arguments"][0] = " --start ";
    auto definition = parse_report_definition(node);
    REQUIRE(std::get<std::string>(definition.m_arguments[0]) == "  ");
    REQUIRE(std::get<OptionalReportArguments>(
      definition.m_arguments[2]).m_arguments[0] == " --start ");
    node = make_definition();
    node["parameters"][0]["type"] = "Unknown";
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["parameters"][0]["default"] = YAML::Load(".nan");
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["parameters"][0]["default"] = YAML::Load("&loop [*loop]");
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
    node = make_definition();
    node["output"]["extension"] = "../csv";
    REQUIRE_THROWS_AS(parse_report_definition(node), std::runtime_error);
  }

  TEST_CASE("unauthenticated_request") {
    auto environment = ServiceLocatorTestEnvironment();
    auto sessions = WebSessionStore<WebPortalSession>();
    auto reports = DefinitionService();
    auto servlet = ReportingWebServlet(Ref(sessions), &reports);
    REQUIRE(reports.m_loads == 0);
    auto request = HttpRequest(
      HttpMethod::POST, Uri("/api/reporting_service/load_report_definitions"));
    REQUIRE(load(servlet, request).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    auto session = sessions.create();
    REQUIRE(load(servlet, make_request(*session)).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    REQUIRE(reports.m_loads == 0);
  }

  TEST_CASE("permitted_definitions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto directory = DirectoryEntry::make_directory(0);
    auto group = client.make_directory("Reporting", directory);
    auto subgroup = client.make_directory("Nested", group);
    auto account = client.make_account("alice", "", subgroup);
    auto other = client.make_account("bob", "", directory);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto definitions = std::vector(
      {make_definition("global", {"*"}), make_definition("account", {"alice"}),
        make_definition("group", {"Reporting"}),
        make_definition("other", {"bob"}),
        make_definition("renamed", {"carol"}), make_definition("none", {})});
    auto reports = LocalReportService(definitions, client, execute,
      TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto servlet = ReportingWebServlet(Ref(sessions), &reports);
    auto session = sessions.create();
    session->set_account(account);
    auto request = make_request(*session);
    auto response = load(servlet, request);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto content_type = response.get_header("Content-Type");
    REQUIRE(static_cast<bool>(content_type));
    REQUIRE(*content_type == "application/json");
    auto values = parse_response(response);
    REQUIRE(values.size() == 3);
    REQUIRE(get<JsonObject>(values[0]).at("id") == "global");
    auto& public_definition = get<JsonObject>(values[0]);
    REQUIRE(!public_definition.get("access"));
    REQUIRE(!public_definition.get("command"));
    REQUIRE(!public_definition.get("arguments"));
    REQUIRE(get<JsonObject>(values[1]).at("id") == "account");
    REQUIRE(get<JsonObject>(values[2]).at("id") == "group");
    client.associate(account, directory);
    client.detach(account, subgroup);
    values = parse_response(load(servlet, request));
    REQUIRE(values.size() == 2);
    REQUIRE(get<JsonObject>(values[0]).at("id") == "global");
    REQUIRE(get<JsonObject>(values[1]).at("id") == "account");
    client.rename(account, "carol");
    values = parse_response(load(servlet, request));
    REQUIRE(values.size() == 2);
    REQUIRE(get<JsonObject>(values[0]).at("id") == "global");
    REQUIRE(get<JsonObject>(values[1]).at("id") == "renamed");
    session->reset_account();
    session->set_account(other);
    values = parse_response(load(servlet, request));
    REQUIRE(values.size() == 2);
    REQUIRE(get<JsonObject>(values[0]).at("id") == "global");
    REQUIRE(get<JsonObject>(values[1]).at("id") == "other");
    auto empty = ReportingWebServlet(Ref(sessions), ReportService(
      std::in_place_type<LocalReportService<decltype(&execute)>>,
      std::vector({make_definition("hidden", {"alice"})}), client, execute,
      TimeClient(std::in_place_type<LocalTimeClient>), 1));
    REQUIRE(parse_response(load(empty, request)).empty());
  }

  TEST_CASE("reload_definitions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto sessions = WebSessionStore<WebPortalSession>();
    auto definitions = std::vector<ReportDefinition>();
    auto reports = LocalReportService(definitions, client, execute,
      TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto servlet = ReportingWebServlet(Ref(sessions), &reports);
    auto session = sessions.create();
    session->set_account(client.get_account());
    auto request = make_request(*session);
    REQUIRE(parse_response(load(servlet, request)).empty());
    definitions.push_back(make_definition("first", {"*"}));
    reports.set_definitions(definitions);
    auto values = parse_response(load(servlet, request));
    REQUIRE(values.size() == 1);
    REQUIRE(get<JsonObject>(values[0]).at("id") == "first");
    definitions[0].m_name = "Updated report";
    definitions[0].m_parameters[2].m_default = JsonValue("CAD");
    definitions.push_back(make_definition("second", {"*"}));
    reports.set_definitions(definitions);
    values = parse_response(load(servlet, request));
    REQUIRE(values.size() == 2);
    REQUIRE(get<JsonObject>(values[0]).at("name") == "Updated report");
    auto& parameters =
      get<std::vector<JsonValue>>(get<JsonObject>(values[0]).at("parameters"));
    REQUIRE(get<JsonObject>(parameters[2]).at("default") == "CAD");
    REQUIRE(get<JsonObject>(values[1]).at("id") == "second");
    definitions[0].m_access = {"other_account"};
    definitions.pop_back();
    reports.set_definitions(definitions);
    REQUIRE(parse_response(load(servlet, request)).empty());
  }
}
