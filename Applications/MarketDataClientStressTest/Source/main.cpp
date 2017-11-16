#include <fstream>
#include <iostream>
#include <random>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/UidService/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "MarketDataClientStressTest/Version.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
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
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  boost::atomic_bool isRunning;

  struct Config {
    vector<Security> m_securities;
    time_duration m_period;
    int m_subscriptionCount;
  };

  struct Subscription {
    Security m_security;
    std::shared_ptr<StateQueue<BboQuote>> m_bboQuotes;
    std::shared_ptr<StateQueue<BookQuote>> m_bookQuotes;
    std::shared_ptr<StateQueue<MarketQuote>> m_marketQuotes;
    std::shared_ptr<StateQueue<TimeAndSale>> m_timeAndSales;
  };

  vector<Security> ParseSecurities(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    vector<Security> securities;
    for(const auto& entry : config) {
      auto security = ParseSecurity(entry.to<string>(), marketDatabase);
      securities.push_back(security);
    }
    return securities;
  }

  void Run(const Config& config, ApplicationMarketDataClient& marketDataClient,
      RefType<TimerThreadPool> timerThreadPool) {
    auto& client = *marketDataClient;
    random_device rd;
    default_random_engine randomizer{rd()};
    std::uniform_int_distribution<std::uint64_t> distribution;
    vector<Subscription> currentSubscriptions;
    int counter = 0;
    while(isRunning.load()) {
      ++counter;
      cout << "Update: " << counter << endl;
      auto unsubscriptionCount =
        distribution(randomizer) % (currentSubscriptions.size() + 1);
      for(auto i = size_t{0}; i != unsubscriptionCount; ++i) {
        auto index =
          int(distribution(randomizer) % currentSubscriptions.size());
        currentSubscriptions.erase(currentSubscriptions.begin() + index);
      }
      auto subscriptionCount = currentSubscriptions.size() +
        distribution(randomizer) %
        (config.m_subscriptionCount - currentSubscriptions.size() + 1);
      while(currentSubscriptions.size() != subscriptionCount) {
        auto index = int(distribution(randomizer) % config.m_securities.size());
        auto& security = config.m_securities[index];
        auto subscriptionIterator = find_if(currentSubscriptions.begin(),
          currentSubscriptions.end(),
          [&] (const Subscription& subscription) {
            return subscription.m_security == security;
          });
        if(subscriptionIterator != currentSubscriptions.end()) {
          continue;
        }
        Subscription subscription;
        subscription.m_security = security;
        subscription.m_bboQuotes = std::make_shared<StateQueue<BboQuote>>();
        client.QueryBboQuotes(BuildCurrentQuery(security),
          subscription.m_bboQuotes);
        subscription.m_bookQuotes = std::make_shared<StateQueue<BookQuote>>();
        QueryRealTimeBookQuotesWithSnapshot(client, security,
          subscription.m_bookQuotes);
        subscription.m_marketQuotes =
          std::make_shared<StateQueue<MarketQuote>>();
        QueryRealTimeMarketQuotesWithSnapshot(client, security,
          subscription.m_marketQuotes);
        subscription.m_timeAndSales =
          std::make_shared<StateQueue<TimeAndSale>>();
        client.QueryTimeAndSales(BuildCurrentQuery(security),
          subscription.m_timeAndSales);
        currentSubscriptions.push_back(subscription);
      }
      auto interval = microseconds(distribution(randomizer) %
        config.m_period.total_microseconds());
      LiveTimer timer(interval, Ref(*timerThreadPool));
      timer.Start();
      timer.Wait();
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" MARKET_DATA_CLIENT_STRESS_TEST_VERSION
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
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  int clientCount;
  Config clientConfig;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    clientConfig.m_period = Extract<time_duration>(config, "period");
    clientConfig.m_securities = ParseSecurities(GetNode(config, "symbols"),
      marketDatabase);
    clientConfig.m_subscriptionCount = std::min<int>(
      Extract<int>(config, "subscription_count"),
      clientConfig.m_securities.size());
    clientCount = Extract<int>(config, "clients");
  } catch(const std::exception& e) {
    cerr << "Unable to initialize client: " << e.what() << endl;
    return -1;
  }
  RoutineHandlerGroup routines;
  isRunning = true;
  for(auto i = 0; i < clientCount; ++i) {
    routines.Spawn(
      [&] {
        ApplicationMarketDataClient marketDataClient;
        try {
          marketDataClient.BuildSession(Ref(*serviceLocatorClient),
            Ref(socketThreadPool), Ref(timerThreadPool));
          marketDataClient->Open();
        } catch(const std::exception&) {
          cerr << "Unable to connect to the market data service." << endl;
        }
        Run(clientConfig, marketDataClient, Ref(timerThreadPool));
      });
  }
  WaitForKillEvent();
  isRunning = false;
  return 0;
}
