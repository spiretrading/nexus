#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/UidService/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "MarketDataClientTemplate/Version.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  vector<Security> ParseSecurities(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    vector<Security> securities;
    for(const auto& entry : config) {
      auto security = ParseSecurity(entry.to<string>(), marketDatabase);
      securities.push_back(security);
    }
    return securities;
  }

  template<typename T>
  void Report(const vector<Security>& securities,
      const vector<std::shared_ptr<Queue<T>>>& queues,
      bool& hasRegression, int& counter) {
    bool checkTimestamp = true;
    for(size_t i = 0; i < queues.size(); ++i) {
      auto& queue = queues[i];
      while(!queue->IsEmpty()) {
        if(checkTimestamp) {
          checkTimestamp = false;
          auto value = queue->Top();
          auto delta = microsec_clock::universal_time() - GetTimestamp(value);
          if(delta >= seconds(8)) {
            cout << "Regression: " << securities[i].GetSymbol() << " " <<
              delta.total_milliseconds() << std::endl;
            hasRegression = true;
          }
        }
        queue->Pop();
        ++counter;
      }
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" MARKET_DATA_CLIENT_TEMPLATE_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      *config.FindValue("service_locator"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  ApplicationAdministrationClient administrationClient;
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    administrationClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error connecting to the administration service: " << e.what() <<
      endl;
    return -1;
  }
  auto marketDataClient = std::make_unique<ApplicationMarketDataClient::Client>(
    BuildMarketDataClientSessionBuilder(Ref(*serviceLocatorClient),
    Ref(socketThreadPool), Ref(timerThreadPool), RELAY_SERVICE_NAME));
  try {
    marketDataClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the market data service." << endl;
    return -1;
  }
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  auto marketDatabase = definitionsClient->LoadMarketDatabase();
  auto securities = ParseSecurities(*config.FindValue("symbols"),
    marketDatabase);
  vector<std::shared_ptr<Queue<BboQuote>>> bboQuotes;
  vector<std::shared_ptr<Queue<MarketQuote>>> marketQuotes;
  vector<std::shared_ptr<Queue<TimeAndSale>>> timeAndSales;
  for(const auto& security : securities) {
    SecurityMarketDataQuery query;
    query.SetIndex(security);
    query.SetRange(Range::RealTime());
    auto bboQueue = std::make_shared<Queue<BboQuote>>();
    marketDataClient->QueryBboQuotes(query, bboQueue);
    bboQuotes.push_back(bboQueue);
    auto marketQueue = std::make_shared<Queue<MarketQuote>>();
    marketDataClient->QueryMarketQuotes(query, marketQueue);
    marketQuotes.push_back(marketQueue);
    auto timeAndSaleQueue = std::make_shared<Queue<TimeAndSale>>();
    marketDataClient->QueryTimeAndSales(query, timeAndSaleQueue);
    timeAndSales.push_back(timeAndSaleQueue);
  }
  bool hasRegression = false;
  while(!ReceivedKillEvent()) {
    if(!hasRegression) {
      this_thread::sleep(seconds{1});
    }
    hasRegression = false;
    int counter = 0;
    Report(securities, bboQuotes, hasRegression, counter);
    Report(securities, marketQuotes, hasRegression, counter);
    Report(securities, timeAndSales, hasRegression, counter);
    std::cout << "Processed: " << counter << " records." << std::endl;
  }
  return 0;
}
