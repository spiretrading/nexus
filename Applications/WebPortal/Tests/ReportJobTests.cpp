#include <sstream>
#include <doctest/doctest.h>
#include "WebPortal/ReportJob.hpp"

using namespace Nexus;

TEST_SUITE("ReportJob") {
  TEST_CASE("status_output") {
    auto output = std::ostringstream();
    output << ReportJob::Status::QUEUED << ' ' << ReportJob::Status::RUNNING <<
      ' ' << ReportJob::Status::COMPLETED << ' ' << ReportJob::Status::FAILED <<
      ' ' << static_cast<ReportJob::Status>(99);
    REQUIRE(output.str() == "QUEUED RUNNING COMPLETED FAILED UNKNOWN(99)");
  }
}
