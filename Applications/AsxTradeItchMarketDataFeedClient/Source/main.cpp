#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Version.hpp"

using namespace Beam;

int main(int argc, const char** argv) {
  try {
    parse_command_line(argc, argv,
      "1.0-r" ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
