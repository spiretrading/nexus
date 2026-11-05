#include <fstream>
#include <future>
#include <Beam/Serialization/SerializationException.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/process/environment.hpp>
#include <boost/scope/scope_exit.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"
#include "WebPortal/FileReportService.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::scope;
using namespace boost::uuids;
using namespace Nexus;
using namespace Nexus::Tests;

TEST_SUITE("ReportFile") {
  TEST_CASE("command_paths") {
    auto directory = std::filesystem::temp_directory_path() /
      to_string(random_generator()());
    auto cleanup = scope_exit([&] {
      std::filesystem::remove_all(directory);
    });
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto service = FileReportService(directory / "definitions", directory,
      client, TimeClient(std::in_place_type<LocalTimeClient>), 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>),
      AdministrationClient(std::in_place_type<TestAdministrationClient>,
        std::make_shared<TestAdministrationClient::Queue>()),
      Timer(std::in_place_type<TriggerTimer>), seconds(5));
#ifdef _WIN32
    auto command = std::string("cmd.exe");
    auto arguments =
      std::vector<std::string>({"/d", "/c", "echo report-command"});
#else
    auto command = std::string("sh");
    auto arguments =
      std::vector<std::string>({"-c", "printf 'report-command\\n'"});
#endif
    auto executable = std::filesystem::path(
      process::environment::find_executable(command).native());
    REQUIRE(!executable.empty());
    auto relative = std::filesystem::relative(
      executable, std::filesystem::current_path());
    REQUIRE(!relative.is_absolute());
    for(auto& path : {command, relative.string()}) {
      auto definition = ReportDefinition("example", "Example", {}, {}, {},
        path, {}, ReportOutputDefinition("text/plain", "txt"));
      auto job = ReportJob({}, client.get_account(), {}, definition, {},
        arguments, time_from_string("2026-11-02 12:00:00"));
      job.m_id = to_string(random_generator()());
      service.store(job);
      REQUIRE(service.execute(job, std::stop_token()) == 0);
      auto output = std::ifstream(directory / job.m_id / "output.txt");
      auto line = std::string();
      std::getline(output, line);
      REQUIRE(line == "report-command");
      job.m_definition.m_command = "nonexistent-report-" + job.m_id;
      REQUIRE_THROWS_AS(
        service.execute(job, std::stop_token()), std::runtime_error);
    }
  }

  TEST_CASE("recovery_notifications") {
    auto directory = std::filesystem::temp_directory_path() /
      to_string(random_generator()());
    auto cleanup = scope_exit([&] {
      std::filesystem::remove_all(directory);
    });
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto time = FixedTimeClient(time_from_string("2026-11-05 12:00:00"));
    auto timer = TriggerTimer();
    auto operations = std::make_shared<TestAdministrationClient::Queue>();
    auto administration = TestAdministrationClient(operations);
    auto definition = ReportDefinition("example", "Example", {}, {}, {},
      "report", {}, ReportOutputDefinition("text/plain", "txt"));
    auto job = ReportJob({}, client.get_account(), {}, definition);
    job.m_id = to_string(random_generator()());
    job.m_status = ReportJob::Status::RUNNING;
    auto loaded = job;
    loaded.m_id = to_string(random_generator()());
    auto old = job;
    old.m_id = to_string(random_generator()());
    old.m_status = ReportJob::Status::FAILED;
    {
      auto service = FileReportService(directory / "definitions", directory,
        client, &time, 1, Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>), &administration,
        Timer(std::in_place_type<TriggerTimer>), seconds(5));
      service.store(job);
      service.store(loaded);
      service.store(old);
    }
    auto service = FileReportService(directory / "definitions", directory,
      client, &time, 1, Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>), &administration, &timer,
      seconds(5));
    REQUIRE(service.load_report(client.get_account(), loaded.m_id).m_status ==
      ReportDetail::Status::FAILED);
    time.set(time.get_time() + seconds(5));
    timer.trigger();
    auto result = *service.load_job(job.m_id);
    REQUIRE(result.m_status == ReportJob::Status::FAILED);
    auto operation = operations->pop();
    auto& notification =
      std::get<TestAdministrationClient::SendNotificationOperation>(*operation);
    notification.m_result.set(Notification());
    REQUIRE(notification.m_data ==
      make_report_notification(result).m_data);
    service.close();
    REQUIRE(!operations->try_pop());
  }

  TEST_CASE("filename_recovery") {
    auto directory = std::filesystem::temp_directory_path() /
      to_string(random_generator()());
    auto cleanup = scope_exit([&] {
      std::filesystem::remove_all(directory);
    });
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = ReportDefinition("example", "Example", {}, {}, {},
      "report", {}, ReportOutputDefinition("text/plain", "bin"));
    auto job = ReportJob({}, client.get_account(), {}, definition, {}, {},
      time_from_string("2026-10-05 12:00:00"),
      time_from_string("2026-10-05 12:01:00"), {},
      ReportJob::Status::COMPLETED);
    job.m_id = to_string(random_generator()());
    {
      auto service = FileReportService(directory / "definitions", directory,
        client, TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>),
        AdministrationClient(std::in_place_type<TestAdministrationClient>,
          std::make_shared<TestAdministrationClient::Queue>()),
        Timer(std::in_place_type<TriggerTimer>), seconds(5));
      service.store(job);
      REQUIRE(service.load_job(job.m_id)->m_filename ==
        "example_2026-10-05.bin");
    }
    auto service = FileReportService(directory / "definitions", directory,
      client, TimeClient(std::in_place_type<LocalTimeClient>), 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>),
      AdministrationClient(std::in_place_type<TestAdministrationClient>,
        std::make_shared<TestAdministrationClient::Queue>()),
      Timer(std::in_place_type<TriggerTimer>), seconds(5));
    service.store(job);
    REQUIRE(service.load_job(job.m_id)->m_filename ==
      "example_2026-10-05.bin");
    job.m_id = to_string(random_generator()());
    auto other = job;
    other.m_id = to_string(random_generator()());
    auto first = std::async(std::launch::async, [&] { service.store(job); });
    auto second = std::async(std::launch::async, [&] { service.store(other); });
    first.get();
    second.get();
    auto names = std::vector({service.load_job(job.m_id)->m_filename,
      service.load_job(other.m_id)->m_filename});
    std::ranges::sort(names);
    REQUIRE(names == std::vector<std::string>({"example_2026-10-05_2.bin",
      "example_2026-10-05_3.bin"}));
  }

  TEST_CASE("metadata_error_context") {
    auto directory = std::filesystem::temp_directory_path() /
      to_string(random_generator()());
    auto cleanup = scope_exit([&] {
      std::filesystem::remove_all(directory);
    });
    auto path = directory;
    SUBCASE("job") {}
    SUBCASE("schedule") {
      path /= "schedules";
    }
    path /= to_string(random_generator()());
    std::filesystem::create_directories(path);
    path /= "metadata.json";
    {
      auto stream = std::ofstream(path);
      stream << "{}";
    }
    auto environment = ServiceLocatorTestEnvironment();
    auto result = try_call([&] {
      auto service = FileReportService(directory / "definitions", directory,
        environment.get_root(), TimeClient(std::in_place_type<LocalTimeClient>),
        1, Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>),
        AdministrationClient(std::in_place_type<TestAdministrationClient>,
          std::make_shared<TestAdministrationClient::Queue>()),
        Timer(std::in_place_type<TriggerTimer>), seconds(5));
    });
    REQUIRE(result.is_exception());
    REQUIRE_THROWS_WITH_AS(result.get(), doctest::Contains(path.string().c_str()),
      std::runtime_error);
    try {
      result.get();
    } catch(const std::runtime_error& error) {
      REQUIRE_THROWS_WITH_AS(std::rethrow_if_nested(error),
        doctest::Contains("\"id\""), SerializationException);
    }
  }
}
