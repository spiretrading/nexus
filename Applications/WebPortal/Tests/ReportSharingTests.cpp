#include <barrier>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportJob make_job(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {}, {}, {}, {},
      ReportOutputDefinition("text/csv", "csv"));
    auto completed = time_from_string("2026-10-06 12:00:00");
    return ReportJob("report", account, {}, definition, {}, {},
      time_from_string("2026-10-01 12:00:00"), completed, completed,
      ReportJob::Status::COMPLETED, 0);
  }

  HttpResponse share(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/share_reports"), from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report sharing route.");
  }
}

TEST_SUITE("ReportSharing") {
  TEST_CASE("endpoint_and_recipient_access") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto root = DirectoryEntry::make_directory(0);
    auto group = client.make_directory("Group", root);
    auto subgroup = client.make_directory("Subgroup", group);
    auto alice = client.make_account("Alice", "", subgroup);
    auto bob = client.make_account("Bob", "", root);
    auto existing = client.make_account("Existing", "", root);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto job = make_job(client.get_account());
    job.m_recipients = {existing};
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("a,b"));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto forged = group;
    forged.m_name = "Forged";
    auto body = JsonObject();
    body["ids"] = std::vector<JsonValue>({job.m_id, job.m_id});
    body["recipients"] = std::vector<JsonValue>({
      parse<JsonValue>(to_json(bob)), parse<JsonValue>(to_json(forged)),
      parse<JsonValue>(to_json(group))});
    auto text = to_string(JsonValue(body));
    REQUIRE(share(servlet, *session, text).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "{}", R"({"ids":[],"recipients":null})",
        R"({"ids":[1],"recipients":[]})", R"({"ids":[],"recipients":[{}]})",
        R"({"ids":[],"recipients":[{"type":0,"id":1.5}]})",
        R"({"ids":[],"recipients":[{"type":2,"id":1}]})"}) {
      REQUIRE(share(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    auto response = share(servlet, *session, text);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE(
      share(servlet, *session, text).get_status_code() == HttpStatusCode::OK);
    auto saved = local.load_job(job.m_id);
    REQUIRE(saved->m_recipients == std::vector({existing, bob, group}));
    REQUIRE(saved->m_recipients.back().m_name == "Group");
    saved->m_recipients = job.m_recipients;
    REQUIRE(to_json(*saved) == to_json(job));
    for(auto& account : {existing, bob, alice}) {
      REQUIRE(local.load_report(account, job.m_id).m_id == job.m_id);
      REQUIRE(
        local.query(account, GeneratedReportQuery()).m_filtered_count == 1);
      REQUIRE(to_string(local.load_file(account, job.m_id).m_content) == "a,b");
    }
    client.associate(alice, root);
    client.detach(alice, subgroup);
    REQUIRE_THROWS_AS(
      local.load_report(alice, job.m_id), ReportNotFoundException);
    auto service = ReportService(&local);
    service.share(client.get_account(), {job.m_id}, {});
    REQUIRE(local.load_job(job.m_id)->m_recipients.size() == 3);
    local.close();
    REQUIRE(share(servlet, *session, text).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("owner_only_and_batch_validation") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto root = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("Alice", "", root);
    auto bob = client.make_account("Bob", "", root);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto service = ReportService(&local);
    auto job = make_job(client.get_account());
    local.store(job);
    auto other = make_job(alice);
    other.m_id = "other";
    other.m_recipients = {client.get_account()};
    local.store(other);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    session->set_account(client.get_account());
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = JsonObject();
    body["recipients"] =
      std::vector<JsonValue>({parse<JsonValue>(to_json(bob))});
    for(auto& id : {"other", "missing"}) {
      body["ids"] = std::vector<JsonValue>({job.m_id, id});
      REQUIRE(share(servlet, *session, to_string(JsonValue(body))).
        get_status_code() == HttpStatusCode::NOT_FOUND);
      REQUIRE(local.load_job(job.m_id)->m_recipients.empty());
    }
    other.m_account = client.get_account();
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::FAILED, ReportJob::Status::CANCELLED}) {
      other.m_status = status;
      local.store(other);
      REQUIRE_THROWS_AS(service.share(client.get_account(),
        {job.m_id, other.m_id}, {bob}), ReportNotFoundException);
      REQUIRE(local.load_job(job.m_id)->m_recipients.empty());
    }
    other.m_status = ReportJob::Status::COMPLETED;
    local.store(other);
    for(auto& invalid : {DirectoryEntry::STAR_DIRECTORY,
        DirectoryEntry::make_account(999999, "Missing"),
        DirectoryEntry::make_directory(bob.m_id, "Wrong type")}) {
      REQUIRE_THROWS_AS(service.share(client.get_account(),
        {job.m_id, other.m_id}, {bob, invalid}), std::invalid_argument);
      REQUIRE(local.load_job(job.m_id)->m_recipients.empty());
      REQUIRE(local.load_job(other.m_id)->m_recipients.size() == 1);
    }
    service.share(client.get_account(), {job.m_id, other.m_id}, {bob});
    REQUIRE(local.load_job(job.m_id)->m_recipients == std::vector({bob}));
    REQUIRE(local.load_job(other.m_id)->m_recipients ==
      std::vector({client.get_account(), bob}));
  }

  TEST_CASE("recipient_permissions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto root = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("Alice", "", root);
    auto bob = client.make_account("Bob", "", root);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto job = make_job(alice);
    local.store(job);
    REQUIRE_THROWS_AS(
      local.share(alice, {job.m_id}, {bob}), std::invalid_argument);
    REQUIRE(local.load_job(job.m_id)->m_recipients.empty());
    client.store(alice, bob, Permission::READ);
    local.share(alice, {job.m_id}, {bob});
    REQUIRE(local.load_report(bob, job.m_id).m_id == job.m_id);
    REQUIRE_THROWS_AS(
      local.share(bob, {job.m_id}, {bob}), ReportNotFoundException);
  }

  TEST_CASE("concurrent_additions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto root = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("Alice", "", root);
    auto bob = client.make_account("Bob", "", root);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto job = make_job(client.get_account());
    local.store(job);
    auto ready = std::barrier(3);
    auto first_error = std::exception_ptr();
    auto second_error = std::exception_ptr();
    auto add = [&] (const auto& recipient, auto& error) {
      ready.arrive_and_wait();
      try {
        local.share(client.get_account(), {job.m_id}, {recipient});
      } catch(const std::exception&) {
        error = std::current_exception();
      }
    };
    auto first = std::jthread([&] { add(alice, first_error); });
    auto second = std::jthread([&] { add(bob, second_error); });
    ready.arrive_and_wait();
    first.join();
    second.join();
    REQUIRE(!first_error);
    REQUIRE(!second_error);
    auto saved = local.load_job(job.m_id);
    REQUIRE(saved->m_recipients.size() == 2);
    REQUIRE(std::ranges::contains(saved->m_recipients, alice));
    REQUIRE(std::ranges::contains(saved->m_recipients, bob));
  }
}
