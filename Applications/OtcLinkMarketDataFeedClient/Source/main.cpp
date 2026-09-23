#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Version.hpp"

using namespace Beam;

int main(int argc, const char** argv) {
  try {
    parse_command_line(argc, argv,
      "1.0-r" OTC_LINK_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
  } catch(...) {
    report_current_exception();
    return -1;
  }
}
