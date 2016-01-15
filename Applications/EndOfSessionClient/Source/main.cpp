#include <fstream>
#include <iostream>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace std;
using namespace TCLAP;

int main(int argc, const char** argv) {
  string configFile;
  vector<string> marketCodes;
  ptime startTime;
  ptime endTime;
  try {
    CmdLine cmd{"", ' ', "0.9-r\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    MultiArg<string> marketArg{"m", "market", "Market to operate on", true,
      "Market identifier code"};
    cmd.add(marketArg);
    ValueArg<string> startTimeArg{"s", "start", "Start time", true, "",
      "date/time"};
    cmd.add(startTimeArg);
    ValueArg<string> endTimeArg{"e", "end", "End time", true, "",
      "date/time"};
    cmd.add(endTimeArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    marketCodes = marketArg.getValue();
    startTime = Parse<DateTimeParser>(startTimeArg.getValue());
    endTime = Parse<DateTimeParser>(endTimeArg.getValue());
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
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  ApplicationOrderExecutionClient orderExecutionClient;
  try {
    orderExecutionClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    orderExecutionClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the order execution service." << endl;
    return -1;
  }
  vector<MarketCode> markets;
  optional<MarketDatabase> marketDatabase;
  try {
    marketDatabase = definitionsClient->LoadMarketDatabase();
  } catch(const std::exception& e) {
    cerr << "Unable to load market database: " << e.what() << endl;
    return -1;
  }
  optional<tz_database> timeZoneDatabase;
  try {
    timeZoneDatabase = definitionsClient->LoadTimeZoneDatabase();
  } catch(const std::exception& e) {
    cerr << "Unable to load time zones: " << e.what() << endl;
    return -1;
  }
  try {
    for(auto& marketCode : marketCodes) {
      auto market = ParseMarketCode(marketCode, *marketDatabase);
      markets.push_back(market);
    }
  } catch(const std::exception& e) {
    cerr << "Unable to parse market codes: " << e.what() << endl;
    return -1;
  }
  optional<vector<DirectoryEntry>> accounts;
  try {
    accounts = serviceLocatorClient->LoadAllAccounts();
  } catch(const std::exception& e) {
    cerr << "Unable to load accounts: " << e.what() << endl;
    return -1;
  }
  for(auto& account : *accounts) {
    for(auto& market : markets) {
      AccountQuery query;
      query.SetIndex(account);
      auto utcStartTime = AdjustDateTime(startTime,
        marketDatabase->FromCode(market).m_timeZone, "UTC",
        *timeZoneDatabase);
      auto utcEndTime = AdjustDateTime(endTime,
        marketDatabase->FromCode(market).m_timeZone, "UTC",
        *timeZoneDatabase);
      query.SetRange(utcStartTime, utcEndTime);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      query.SetFilter(BuildMarketFilter(market));
      auto orderQueue = std::make_shared<Queue<const Order*>>();
      orderExecutionClient->QueryOrderSubmissions(query, orderQueue);
      vector<const Order*> orders;
      FlushQueue(orderQueue, back_inserter(orders));
      for(auto& order : orders) {
        bool isTerminal;
        order->GetPublisher().WithSnapshot(
          [&] (optional<const vector<ExecutionReport>&> executionReports) {
            if(!executionReports.is_initialized() ||
                executionReports->empty()) {
              isTerminal = false;
              return;
            }
            isTerminal = IsTerminal(executionReports->back().m_status);
          });
        if(!isTerminal) {
          ExecutionReport executionReport;
          executionReport.m_status = OrderStatus::CANCELED;
          executionReport.m_text = "End of session.";
          orderExecutionClient->Update(order->GetInfo().m_orderId,
            executionReport);
        }
      }
    }
  }
  return 0;
}
