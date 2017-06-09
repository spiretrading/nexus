#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "OrderExecutionBackup/Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace std;
using namespace TCLAP;

namespace {
  void StoreOrderRecords(const DirectoryEntry& account, const ptime& startTime,
      const ptime& endTime,
      ApplicationOrderExecutionClient& orderExecutionClient,
      LocalOrderExecutionDataStore& dataStore) {
    AccountQuery query;
    query.SetIndex(account);
    query.SetRange(startTime, endTime);
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<OrderRecord>>();
    orderExecutionClient->QueryOrderRecords(query, queue);
    auto sequence =
      LoadInitialSequences(dataStore, account).m_nextOrderInfoSequence;
    try {
      while(true) {
        auto orderRecord = queue->Top();
        queue->Pop();
        auto sequencedOrderInfo = MakeSequencedValue(
          MakeIndexedValue(orderRecord.m_info, account), sequence);
        ++sequence;
        dataStore.Store(sequencedOrderInfo);
      }
    } catch(const std::exception&) {}
  }

  void StoreExecutionReports(const DirectoryEntry& account,
      const ptime& startTime, const ptime& endTime,
      ApplicationOrderExecutionClient& orderExecutionClient,
      LocalOrderExecutionDataStore& dataStore) {
    AccountQuery query;
    query.SetIndex(account);
    query.SetRange(startTime, endTime);
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<ExecutionReport>>();
    orderExecutionClient->QueryExecutionReports(query, queue);
    auto sequence =
      LoadInitialSequences(dataStore, account).m_nextExecutionReportSequence;
    try {
      while(true) {
        auto executionReport = queue->Top();
        queue->Pop();
        auto sequencedExecutionReport = MakeSequencedValue(
          MakeIndexedValue(executionReport, account), sequence);
        ++sequence;
        dataStore.Store(sequencedExecutionReport);
      }
    } catch(const std::exception&) {}
  }

  void SaveOrderRecords(const LocalOrderExecutionDataStore& dataStore,
      BinarySender<SharedBuffer>& sender) {
    auto orderRecords = dataStore.LoadOrderSubmissions();
    unordered_map<DirectoryEntry, vector<SequencedOrderRecord>>
      sequencedAccountOrderRecords;
    for(const auto& orderRecord : orderRecords) {
      auto& entry = sequencedAccountOrderRecords[orderRecord->GetIndex()];
      auto insertIterator = std::lower_bound(entry.begin(), entry.end(),
        orderRecord, SequenceComparator());
      entry.insert(insertIterator, orderRecord);
    }
    unordered_map<DirectoryEntry, vector<OrderRecord>> accountOrderRecords;
    for(const auto& entry : sequencedAccountOrderRecords) {
      const auto& account = entry.first;
      const auto& orderRecords = entry.second;
      std::transform(orderRecords.begin(), orderRecords.end(),
        std::back_inserter(accountOrderRecords[entry.first]),
        [] (const SequencedOrderRecord& orderRecord) {
          return *orderRecord;
        });
    }
    sender.Shuttle(accountOrderRecords);
  }

  void SaveExecutionReports(const LocalOrderExecutionDataStore& dataStore,
      BinarySender<SharedBuffer>& sender) {
    auto executionReports = dataStore.LoadExecutionReports();
    unordered_map<DirectoryEntry, vector<SequencedExecutionReport>>
      sequencedAccountExecutionReports;
    for(const auto& executionReport : executionReports) {
      auto& entry = sequencedAccountExecutionReports[
        executionReport->GetIndex()];
      auto insertIterator = std::lower_bound(entry.begin(), entry.end(),
        executionReport, SequenceComparator());
      entry.insert(insertIterator, executionReport);
    }
    unordered_map<DirectoryEntry, vector<ExecutionReport>>
      accountExecutionReports;
    for(const auto& entry : sequencedAccountExecutionReports) {
      const auto& account = entry.first;
      const auto& executionReports = entry.second;
      std::transform(executionReports.begin(), executionReports.end(),
        std::back_inserter(accountExecutionReports[entry.first]),
        [] (const SequencedExecutionReport& executionReport) {
          return *executionReport;
        });
    }
    sender.Shuttle(accountExecutionReports);
  }

  void Save(const LocalOrderExecutionDataStore& dataStore,
      const path& backupPath) {
    try {
      BinarySender<SharedBuffer> sender;
      SharedBuffer buffer;
      sender.SetSink(Ref(buffer));
      SaveOrderRecords(dataStore, sender);
      SaveExecutionReports(dataStore, sender);
      BasicOStreamWriter<filesystem::ofstream> writer(
        Initialize(backupPath, ios::binary));
      writer.Write(buffer);
    } catch(const std::exception& e) {
      cout << "Error saving backup: " << e.what() << endl;
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  ptime startTime;
  ptime endTime;
  try {
    CmdLine cmd{"", ' ', "0.9-r" ORDER_EXECUTION_BACKUP_VERSION
      "\nCopyright (C) 2014 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    ValueArg<string> startArg{"s", "start", "Start Time", true, "", "text"};
    ValueArg<string> endArg{"e", "end", "End Time", true, "", "text"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    startTime = ToUtcTime(Parse<DateTimeParser>(startArg.getValue()));
    endTime = ToUtcTime(Parse<DateTimeParser>(endArg.getValue()));
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    std::ifstream configStream{configFile.c_str()};
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
      GetNode(config, "service_locator"));
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
  ApplicationOrderExecutionClient orderExecutionClient;
  try {
    orderExecutionClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    orderExecutionClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the order execution service." << endl;
    return -1;
  }
  LocalOrderExecutionDataStore dataStore;
  vector<DirectoryEntry> accounts;
  try {
    accounts = serviceLocatorClient->LoadAllAccounts();
  } catch(const std::exception& e) {
    cerr << "Unable to load accounts: " << e.what() << endl;
    return -1;
  }
  for(const auto& account : accounts) {
    StoreOrderRecords(account, startTime, endTime, orderExecutionClient,
      dataStore);
    StoreExecutionReports(account, startTime, endTime, orderExecutionClient,
      dataStore);
  }
  path backupPath = to_iso_string(startTime) + "-" +
    to_iso_string(endTime) + ".dat";
  Save(dataStore, backupPath);
  return 0;
}
