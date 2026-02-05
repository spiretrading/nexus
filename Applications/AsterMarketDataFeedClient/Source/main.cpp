#include <iostream>
#include <Beam/Queues/Queue.hpp>
#include <Beam/WebServices/TcpChannelFactory.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include "AsterMarketDataFeedClient/AsterWebClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;

int main(int argc, const char** argv) {
  auto client = AsterWebClient(TcpSocketChannelFactory());
  client.ping();
  std::cout << client.load_server_time() << std::endl;
  auto data = std::make_shared<Queue<TimeAndSale>>();
  client.subscribe(Security("BTCUSDT", Venue("ASTR")), data);
  while(!received_kill_event()) {
    auto item = data->pop();
    std::cout << item << std::endl;
  }
  return 0;
}
