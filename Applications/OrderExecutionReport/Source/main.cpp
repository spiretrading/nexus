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

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace std;
using namespace TCLAP;

namespace {
  struct ActivityLogEntry {
    OrderFields m_fields;
    ExecutionReport m_report;
  };

  void OutputOrderLog(const vector<const Order*>& orders) {
    for(const auto& order : orders) {
      cout << ToLocalTime(order->GetInfo().m_timestamp) << "," <<
        order->GetInfo().m_fields.m_account.m_name << "," <<
        order->GetInfo().m_orderId << "," <<
        order->GetInfo().m_fields.m_security.GetSymbol() << "," <<
        ToString(order->GetInfo().m_fields.m_type) << "," <<
        ToString(order->GetInfo().m_fields.m_side) << "," <<
        order->GetInfo().m_fields.m_destination << "," <<
        order->GetInfo().m_fields.m_quantity << "," <<
        ToString(order->GetInfo().m_fields.m_price) << "," <<
        ToString(order->GetInfo().m_fields.m_timeInForce.GetType()) << endl;
    }
  }

  void OutputActivityLog(const vector<ActivityLogEntry>& executionReports) {
    for(const auto& executionReport : executionReports) {
      cout << ToLocalTime(executionReport.m_report.m_timestamp) << "," <<
        executionReport.m_report.m_id << "," <<
        executionReport.m_fields.m_side << "," <<
        executionReport.m_fields.m_security.GetSymbol() << "," <<
        ToString(executionReport.m_report.m_status) << "," <<
        executionReport.m_report.m_lastQuantity << "," <<
        executionReport.m_report.m_lastPrice << "," <<
        executionReport.m_report.m_lastMarket << "," <<
        executionReport.m_report.m_liquidityFlag << endl;
    }
  }
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
  ApplicationOrderExecutionClient orderExecutionClient;
  try {
    orderExecutionClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    orderExecutionClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the order execution service." << endl;
    return -1;
  }
  auto startDate = Extract<ptime>(config, "start");
  auto endDate = Extract<ptime>(config, "end");
  auto accountName = Extract<string>(config, "account", "");
  vector<const Order*> orders;
  vector<ActivityLogEntry> activityLog;
  if(!accountName.empty()) {
    auto account = serviceLocatorClient->FindAccount(accountName);
    if(!account.is_initialized()) {
      cerr << "Account not found: " << accountName;
      return -1;
    }
    AccountQuery query;
    query.SetIndex(*account);
    query.SetRange(ToUtcTime(startDate), ToUtcTime(endDate));
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto orderQueue = std::make_shared<Queue<const Order*>>();
    orderExecutionClient->QueryOrderSubmissions(query, orderQueue);
    FlushQueue(orderQueue, back_inserter(orders));
    for(auto& order : orders) {
      order->GetPublisher().WithSnapshot(
        [&] (optional<const vector<ExecutionReport>&> orderReports) {
          if(orderReports.is_initialized()) {
            for(const auto& orderReport : *orderReports) {
              activityLog.push_back({order->GetInfo().m_fields, orderReport});
            }
          }
        });
    }
  } else {
    auto security = ParseSecurity(Extract<string>(config, "security"),
      definitionsClient->LoadMarketDatabase());
    auto accounts = serviceLocatorClient->LoadAllAccounts();
    for(const auto& account : accounts) {
      AccountQuery query;
      query.SetIndex(account);
      query.SetRange(ToUtcTime(startDate), ToUtcTime(endDate));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto orderQueue = std::make_shared<Queue<const Order*>>();
      orderExecutionClient->QueryOrderSubmissions(query, orderQueue);
      vector<const Order*> accountOrders;
      vector<ActivityLogEntry> accountActivityLog;
      FlushQueue(orderQueue, back_inserter(accountOrders));
      auto removeIterator = std::remove_if(accountOrders.begin(),
        accountOrders.end(),
        [&] (const Order* order) {
          return order->GetInfo().m_fields.m_security != security;
        });
      accountOrders.erase(removeIterator, accountOrders.end());
      for(auto& order : accountOrders) {
        order->GetPublisher().WithSnapshot(
          [&] (optional<const vector<ExecutionReport>&> orderReports) {
            if(orderReports.is_initialized()) {
              for(const auto& orderReport : *orderReports) {
                accountActivityLog.push_back({order->GetInfo().m_fields,
                  orderReport});
              }
            }
          });
      }
      orders.insert(orders.end(), accountOrders.begin(), accountOrders.end());
      activityLog.insert(activityLog.end(), accountActivityLog.begin(),
        accountActivityLog.end());
    }
  }
  sort(orders.begin(), orders.end(),
    [] (const Order* lhs, const Order* rhs) {
      return lhs->GetInfo().m_timestamp < rhs->GetInfo().m_timestamp;
    });
  sort(activityLog.begin(), activityLog.end(),
    [] (const ActivityLogEntry& lhs, const ActivityLogEntry& rhs) {
      return lhs.m_report.m_timestamp < rhs.m_report.m_timestamp;
    });
  OutputOrderLog(orders);
  OutputActivityLog(activityLog);
  return 0;
}
