#include <future>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/scope/scope_exit.hpp>
#include <doctest/doctest.h>
#include "WebPortal/FileReportService.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("ReportFile") {
  TEST_CASE("filename_recovery") {
    auto directory = std::filesystem::temp_directory_path() /
      boost::uuids::to_string(boost::uuids::random_generator()());
    auto cleanup = boost::scope::scope_exit([&] {
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
    job.m_id = boost::uuids::to_string(boost::uuids::random_generator()());
    {
      auto service = FileReportService(directory / "definitions", directory,
        client, TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
      service.store(job);
      REQUIRE(service.load_job(job.m_id)->m_filename ==
        "example_2026-10-05.bin");
    }
    auto service = FileReportService(directory / "definitions", directory,
      client, TimeClient(std::in_place_type<LocalTimeClient>), 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    service.store(job);
    REQUIRE(service.load_job(job.m_id)->m_filename ==
      "example_2026-10-05.bin");
    job.m_id = boost::uuids::to_string(boost::uuids::random_generator()());
    auto other = job;
    other.m_id = boost::uuids::to_string(boost::uuids::random_generator()());
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

}
