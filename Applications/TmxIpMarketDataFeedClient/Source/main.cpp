#include <tclap/CmdLine.h>
#include <tclap/StdOutput.h>
#include "Version.hpp"

int main(int argc, const char** argv) {
  auto command_line = TCLAP::CmdLine("TMX IP market data feed client.", ' ',
    "1.0-r" TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION
    "\nCopyright (C) 2026 Spire Trading Inc.");
  command_line.parse(argc, argv);
  TCLAP::StdOutput().usage(command_line);
  return 0;
}
