#include <fstream>
#include <iostream>
#include <set>
#include <unordered_map>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/range/adaptor/map.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace std;
using namespace TCLAP;

namespace {
  using ApplicationPortfolio =
    Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>;
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r\nCopyright (C) 2009 Eidolon Systems Ltd."};
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
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  ApplicationAdministrationClient administrationClient;
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    administrationClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the administration service." << endl;
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
  ptime startDate;
  ptime endDate;
  MarketDatabase marketDatabase;
  vector<DirectoryEntry> tradingGroupEntries;
  try {
    startDate = Extract<ptime>(config, "start");
    endDate = Extract<ptime>(config, "end");
    marketDatabase = definitionsClient->LoadMarketDatabase();
    tradingGroupEntries = administrationClient->LoadManagedTradingGroups(
      serviceLocatorClient->GetAccount());
  } catch(const std::exception& e) {
    cerr << "Unable to initialize report: " << e.what() << endl;
    return -1;
  }
  for(auto& tradingGroupEntry : tradingGroupEntries) {
    TradingGroup tradingGroup;
    try {
      tradingGroup = administrationClient->LoadTradingGroup(tradingGroupEntry);
    } catch(const std::exception& e) {
      cerr << "Unable to load trading group '" << tradingGroupEntry.m_name <<
        "': " << e.what() << endl;
      continue;
    }
    vector<CurrencyId> currencies;
    currencies.push_back(DefaultCurrencies::AUD());
    currencies.push_back(DefaultCurrencies::CAD());
    unordered_map<CurrencyId, Money> groupProfitAndLoss;
    cout << tradingGroupEntry.m_name << endl;
    for(auto& account : tradingGroup.GetTraders()) {
      ApplicationPortfolio accountPortfolio{marketDatabase};
      unordered_map<CurrencyId, Quantity> volumes;
      AccountQuery query;
      query.SetIndex(account);
      query.SetRange(ToUtcTime(startDate), ToUtcTime(endDate));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto orderQueue = std::make_shared<Queue<const Order*>>();
      orderExecutionClient->QueryOrderSubmissions(query, orderQueue);
      try {
        while(true) {
          auto order = orderQueue->Top();
          orderQueue->Pop();
          order->GetPublisher().WithSnapshot(
            [&] (boost::optional<const std::vector<ExecutionReport>&>
                executionReports) {
              for(auto& executionReport : *executionReports) {
                accountPortfolio.Update(order->GetInfo().m_fields,
                  executionReport);
                volumes[order->GetInfo().m_fields.m_currency] +=
                  executionReport.m_lastQuantity;
              }
            });
        }
      } catch(const PipeBrokenException&) {}
      if(!volumes.empty()) {
        cout << "\t" << account.m_name << endl;
        for(auto currency : currencies) {
          if(volumes[currency] != 0) {
            auto accountTotals = accountPortfolio.GetBookkeeper().GetTotal(
              currency);
            auto netProfitAndLoss = GetRealizedProfitAndLoss(accountTotals);
            groupProfitAndLoss[currency] += netProfitAndLoss;
            cout << "\t\tCurrency: " <<
              GetDefaultCurrencyDatabase().FromId(currency).m_code << endl;
            cout << "\t\t\tVolume: " << volumes[currency] << endl;
            cout << "\t\t\tP/L " << netProfitAndLoss.ToString() << endl << endl;
          }
        }
      }
    }
    if(!groupProfitAndLoss.empty()) {
      cout << "\tTotals" << endl;
      for(auto currency : currencies) {
        cout << "\t\t" <<
          GetDefaultCurrencyDatabase().FromId(currency).m_code << ": " <<
          groupProfitAndLoss[currency].ToString() << endl;
      }
      cout << endl;
    }
  }
  return 0;
}
