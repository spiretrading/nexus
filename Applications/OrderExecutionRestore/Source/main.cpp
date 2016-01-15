#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/OrderExecutionService/MySqlOrderExecutionDataStore.hpp"
#include "OrderExecutionRestore/Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::MySql;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace std;
using namespace TCLAP;

namespace {
  void RestoreOrderRecords(BinaryReceiver<SharedBuffer>& receiver,
      MySqlOrderExecutionDataStore& targetDataStore) {
    unordered_map<DirectoryEntry, vector<OrderRecord>> orderRecords;
    receiver.Shuttle(orderRecords);
    for(const auto& entry : orderRecords) {
      const auto& account = entry.first;
      const auto& orderRecords = entry.second;
      auto sequence =
        targetDataStore.LoadInitialSequences(account).m_nextOrderInfoSequence;
      vector<SequencedAccountOrderInfo> sequencedOrderInfo;
      for(const auto& orderRecord : orderRecords) {
        auto entry = MakeSequencedValue(
          MakeIndexedValue(orderRecord.m_info, account), sequence);
        sequencedOrderInfo.push_back(entry);
        ++sequence;
      }
      targetDataStore.Store(sequencedOrderInfo);
    }
  }

  void RestoreExecutionReports(BinaryReceiver<SharedBuffer>& receiver,
      MySqlOrderExecutionDataStore& targetDataStore) {
    unordered_map<DirectoryEntry, vector<ExecutionReport>> executionReports;
    receiver.Shuttle(executionReports);
    for(const auto& entry : executionReports) {
      const auto& account = entry.first;
      const auto& executionReports = entry.second;
      auto sequence = targetDataStore.LoadInitialSequences(
        account).m_nextExecutionReportSequence;
      vector<SequencedAccountExecutionReport> sequencedExecutionReports;
      for(const auto& executionReport : executionReports) {
        auto sequencedExecutionReport = MakeSequencedValue(
          MakeIndexedValue(executionReport, account), sequence);
        sequencedExecutionReports.push_back(sequencedExecutionReport);
        ++sequence;
      }
      targetDataStore.Store(sequencedExecutionReports);
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  string backupFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" ORDER_EXECUTION_RESTORE_VERSION
      "\nCopyright (C) 2014 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    ValueArg<string> backupArg{"b", "backup", "Backup file", true, "", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    backupFile = backupArg.getValue();
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
  MySqlConfig mySqlConfig;
  try {
    mySqlConfig = MySqlConfig::Parse(*config.FindValue("data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  MySqlOrderExecutionDataStore targetDataStore{mySqlConfig.m_address,
    mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password};
  try {
    targetDataStore.Open();
    BasicIStreamReader<filesystem::ifstream> reader{
      Initialize(backupFile, ios::binary)};
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    BinaryReceiver<SharedBuffer> receiver;
    receiver.SetSource(Ref(buffer));
    RestoreOrderRecords(receiver, targetDataStore);
    RestoreExecutionReports(receiver, targetDataStore);
  } catch(const std::exception& e) {
    cout << "Unable to load backup:" << e.what() << endl;
  }
  return 0;
}
