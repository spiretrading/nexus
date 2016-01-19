#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/range/adaptor/map.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace std;
using namespace TCLAP;

namespace {
  using ApplicationPortfolio =
    Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>;

  boost::posix_time::ptime ConvertTimeZone(
      const boost::posix_time::ptime& source, const std::string& sourceTimeZone,
      const std::string& targetTimeZone,
      const boost::local_time::tz_database& timeZoneDatabase) {
    auto sourceTimeZoneEntry = timeZoneDatabase.time_zone_from_region(
      sourceTimeZone);
    auto targetTimeZoneEntry = timeZoneDatabase.time_zone_from_region(
      targetTimeZone);
    boost::local_time::local_date_time sourceDateTime{source.date(),
      source.time_of_day(), sourceTimeZoneEntry,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR};
    auto result = sourceDateTime.local_time_in(targetTimeZoneEntry);
    return {result.date(), result.time_of_day()};
  }

  void QueryDailyOrderSubmissions(const DirectoryEntry& account,
      const ptime& startTime, CountryCode country,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      ApplicationOrderExecutionClient& orderExecutionClient,
      const std::shared_ptr<QueueWriter<const Order*>>& queue) {
    Spawn(
      [account, startTime, country, &marketDatabase, &timeZoneDatabase,
          &orderExecutionClient, queue] {
        for(auto& market : marketDatabase.GetEntries()) {
          if(market.m_countryCode != country) {
            continue;
          }
          auto snapshotQuery = BuildDailyOrderSubmissionQuery(market.m_code,
            account, startTime, startTime, marketDatabase, timeZoneDatabase);
          auto snapshotQueue = std::make_shared<Queue<SequencedOrder>>();
          orderExecutionClient->QueryOrderSubmissions(snapshotQuery,
            snapshotQueue);
          try {
            while(true) {
              queue->Push(snapshotQueue->Top().GetValue());
              snapshotQueue->Pop();
            }
          } catch(const std::exception&) {}
        }
        queue->Break();
      });
  }

  void ExecuteAustralianReport(ptime startDate, ptime endDate,
      const vector<DirectoryEntry>& accounts,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      ApplicationOrderExecutionClient& orderExecutionClient) {
    const auto SPIRE_CHARGES = 18 * Money::CENT;
    const auto SPIRE_COLLECTION = 18 * Money::CENT;
    const auto CLEARING_FEE = boost::rational<int>{125, 1000000};
    const auto CLEARING_COLLECTION = boost::rational<int>{125, 1000000};
    const auto PAYOUT = rational<int>{80, 100};
    ApplicationPortfolio portfolio{marketDatabase};
    auto notionalValue = Money::ZERO;
    auto executionCharges = Money::ZERO;
    auto spireCharges = Money::ZERO;
    auto spireCollections = Money::ZERO;
    auto clearingCharges = Money::ZERO;
    auto clearingCollections = Money::ZERO;
    auto trades = 0;
    auto dayCount = 0;
    unordered_map<DirectoryEntry, ApplicationPortfolio> accountPortfolios;
    unordered_map<MarketCode, Quantity> destinationVolume;
    startDate = ConvertTimeZone(startDate,
      marketDatabase.FromCode(DefaultMarkets::ASX()).m_timeZone, "UTC",
      timeZoneDatabase);
    endDate = ConvertTimeZone(endDate,
      marketDatabase.FromCode(DefaultMarkets::ASX()).m_timeZone, "UTC",
      timeZoneDatabase);
    while(startDate <= endDate) {
      auto marketOpen = false;
      for(const auto& account : accounts) {
        if(account.m_name == "tradeout") {
          continue;
        }
        auto orderQueue = std::make_shared<Queue<const Order*>>();
        QueryDailyOrderSubmissions(account, startDate, DefaultCountries::AU(),
          marketDatabase, timeZoneDatabase, orderExecutionClient, orderQueue);
        try {
          while(true) {
            auto order = orderQueue->Top();
            marketOpen = true;
            orderQueue->Pop();
            order->GetPublisher().WithSnapshot(
              [&] (boost::optional<const std::vector<ExecutionReport>&>
                  executionReports) {
                for(const auto& executionReport : *executionReports) {
                  portfolio.Update(order->GetInfo().m_fields, executionReport);
                  GetOrInsert(accountPortfolios, account,
                    [&] {
                      return ApplicationPortfolio(marketDatabase);
                    }).Update(order->GetInfo().m_fields, executionReport);
                  if(executionReport.m_lastQuantity != 0) {
                    ++trades;
                    destinationVolume[executionReport.m_lastMarket] +=
                      executionReport.m_lastQuantity;
                    executionCharges += executionReport.m_executionFee;
                    notionalValue += executionReport.m_lastQuantity *
                      executionReport.m_lastPrice;
                    spireCollections += SPIRE_COLLECTION;
                    spireCharges += SPIRE_CHARGES;
                    clearingCollections += CLEARING_FEE *
                      (executionReport.m_lastQuantity *
                      executionReport.m_lastPrice);
                    clearingCharges += CLEARING_FEE *
                      (executionReport.m_lastQuantity *
                      executionReport.m_lastPrice);
                  }
                }
              });
          }
        } catch(const PipeBrokenException&) {}
      }
      startDate += gregorian::days(1);
      if(marketOpen) {
        ++dayCount;
      }
    }
    if(dayCount == 0) {
      dayCount = 1;
    }
    cout << "Australian Report\n";
    cout << "-------------------\n\n";
    cout << "Open Positions" << endl;
    for(const auto& accountPortfolio : accountPortfolios) {
      bool isEmpty = true;
      std::stringstream output;
      output << "\t " << accountPortfolio.first.m_name << endl;
      for(const auto& inventory :
          accountPortfolio.second.GetBookkeeper().GetInventoryRange()) {
        if(inventory.second.m_position.m_quantity != 0) {
          output << "\t\t " <<
            ToString(inventory.first.m_index, marketDatabase) << " " <<
            inventory.second.m_position.m_quantity << " " <<
            ToString(GetAveragePrice(inventory.second.m_position)) << endl;
          isEmpty = false;
        }
      }
      output << endl;
      if(!isEmpty) {
        cout << output.str();
      }
    }
    cout << endl;
    cout << "Destination Volume" << endl;
    for(auto i : destinationVolume) {
      cout << "\t" << i.first.GetData() << "\t" << i.second << endl;
    }
    cout << endl;
    auto totals = portfolio.GetBookkeeper().GetTotal(DefaultCurrencies::AUD());
    auto netProfitAndLoss = (1 - PAYOUT) * GetRealizedProfitAndLoss(totals);
    auto netFees = (clearingCollections - clearingCharges) +
      (spireCollections - spireCharges);
    auto totalNet = netProfitAndLoss + netFees;
    cout << "Daily Averages" << endl;
    cout << "Spire Fee          : " << (spireCharges / dayCount).ToString() <<
      endl;
    cout << "Spire Collections  : " <<
      (spireCollections / dayCount).ToString() << endl;
    cout << "Clearing Fee       : " <<
      (clearingCharges / dayCount).ToString() << endl;
    cout << "Clearing Collection: " <<
      (clearingCollections / dayCount).ToString() << endl;
    cout << "Execution Fee      : " <<
      (executionCharges / dayCount).ToString() << endl;
    cout << "Profit and Loss    : " <<
      (netProfitAndLoss / dayCount).ToString() << endl;
    cout << "Net Fees           : " << (netFees / dayCount).ToString() << endl;
    cout << "Total Net          : " << (totalNet / dayCount).ToString() << endl;
    cout << "Volume             : " << (totals.m_volume / dayCount) << endl;
    cout << "Notional Value     : " << (notionalValue / dayCount).ToString() <<
      endl;
    cout << "Trades             : " << (trades / dayCount) << endl;
    cout << endl << endl;
    cout << "Totals" << endl;
    cout << "Spire Fee          : " << spireCharges.ToString() << endl;
    cout << "Spire Collections  : " << spireCollections.ToString() << endl;
    cout << "Clearing Fee       : " << clearingCharges.ToString() << endl;
    cout << "Clearing Collect   : " << clearingCollections.ToString() << endl;
    cout << "Execution Fee      : " << executionCharges.ToString() << endl;
    cout << "Profit and Loss    : " << netProfitAndLoss.ToString() << endl;
    cout << "Net Fees           : " << netFees.ToString() << endl;
    cout << "Total Net          : " << totalNet.ToString() << endl;
    cout << "Volume             : " << totals.m_volume << endl;
    cout << "Notional Value     : " << notionalValue.ToString() << endl;
    cout << "Trades             : " << trades << endl;
  }

  void ExecuteCanadianReport(ptime startDate, ptime endDate,
      const vector<DirectoryEntry>& accounts,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      ApplicationOrderExecutionClient& orderExecutionClient) {
    const auto SPIRE_FEE = (14 * Money::BIP) / 10;
    const auto SPIRE_COLLECTION = (14 * Money::BIP) / 10;
    const auto IIROC_FEE = (25 * Money::ONE) / 1000000;
    const auto IIROC_COLLECTION = (132 * Money::CENT) / 10;
    const auto CDS_FEE = (825 * Money::ONE) / 100000;
    const auto CDS_COLLECTION = 13 * Money::CENT;
    const auto CDS_COLLECTION_CAP = 5;
    const auto PER_ORDER_COLLECTION = Money::BIP;
    const auto PER_ORDER_COLLECTION_CAP = 10 * Money::CENT;
    const auto TICKET_FEE = 3 * Money::ONE + 50 * Money::CENT;
    const auto SUB_DOLLAR_CLEARING_FEE = Money::BIP;
    const auto CLEARING_FEE = (125 * Money::BIP) / 100;
    const auto CLEARING_COLLECTION = Money::BIP;
    const auto PAYOUT = rational<int>{93, 100};
    ApplicationPortfolio portfolio{marketDatabase};
    auto notionalValue = Money::ZERO;
    auto ticketCharges = Money::ZERO;
    auto executionCharges = Money::ZERO;
    auto cdsCharges = Money::ZERO;
    auto cdsCollections = Money::ZERO;
    auto iirocCharges = Money::ZERO;
    auto iirocCollections = Money::ZERO;
    auto orderCollections = Money::ZERO;
    auto spireCharges = Money::ZERO;
    auto spireCollections = Money::ZERO;
    auto clearingCharges = Money::ZERO;
    auto clearingCollections = Money::ZERO;
    auto trades = 0;
    unordered_map<OrderId, int> orderFillCounts;
    auto dayCount = 0;
    unordered_map<DirectoryEntry, ApplicationPortfolio> accountPortfolios;
    unordered_map<MarketCode, Quantity> destinationVolume;
    startDate = ConvertTimeZone(startDate,
      marketDatabase.FromCode(DefaultMarkets::TSX()).m_timeZone, "UTC",
      timeZoneDatabase);
    endDate = ConvertTimeZone(endDate,
      marketDatabase.FromCode(DefaultMarkets::TSX()).m_timeZone, "UTC",
      timeZoneDatabase);
    while(startDate <= endDate) {
      set<Security> securities;
      unordered_map<const Order*, Money> orderFeeTab;
      for(const auto& account : accounts) {
        if(account.m_name == "tradeout") {
          continue;
        }
        auto orderQueue = std::make_shared<Queue<const Order*>>();
        QueryDailyOrderSubmissions(account, startDate, DefaultCountries::CA(),
          marketDatabase, timeZoneDatabase, orderExecutionClient, orderQueue);
        try {
          while(true) {
            auto order = orderQueue->Top();
            orderQueue->Pop();
            order->GetPublisher().WithSnapshot(
              [&] (boost::optional<const std::vector<ExecutionReport>&>
                  executionReports) {
                for(const auto& executionReport : *executionReports) {
                  portfolio.Update(order->GetInfo().m_fields, executionReport);
                  GetOrInsert(accountPortfolios, account,
                    [&] {
                      return ApplicationPortfolio(marketDatabase);
                    }).Update(order->GetInfo().m_fields, executionReport);
                  if(executionReport.m_lastQuantity != 0) {
                    auto& orderFillCount =
                      orderFillCounts[executionReport.m_id];
                    ++orderFillCount;
                    ++trades;
                    destinationVolume[executionReport.m_lastMarket] +=
                      executionReport.m_lastQuantity;
                    if(securities.insert(
                        order->GetInfo().m_fields.m_security).second) {
                      ticketCharges += TICKET_FEE;
                    }
                    cdsCharges += CDS_FEE;
                    iirocCharges += executionReport.m_lastQuantity * IIROC_FEE;
                    if(orderFillCount <= CDS_COLLECTION_CAP) {
                      cdsCollections += CDS_COLLECTION;
                    }
                    iirocCollections += IIROC_COLLECTION;
                    executionCharges += executionReport.m_executionFee;
                    notionalValue += executionReport.m_lastQuantity *
                      executionReport.m_lastPrice;
                    auto& tab = orderFeeTab[order];
                    auto delta = executionReport.m_lastQuantity *
                      PER_ORDER_COLLECTION;
                    if(tab + delta > 10 * Money::CENT) {
                      delta = 10 * Money::CENT - tab;
                    }
                    tab += delta;
                    orderCollections += delta;
                    spireCollections += executionReport.m_lastQuantity *
                      SPIRE_COLLECTION;
                    spireCharges += executionReport.m_lastQuantity *
                      SPIRE_COLLECTION;
                    clearingCollections += executionReport.m_lastQuantity *
                      CLEARING_COLLECTION;
                    if(executionReport.m_lastPrice < Money::ONE) {
                      clearingCharges += executionReport.m_lastQuantity *
                        SUB_DOLLAR_CLEARING_FEE;
                    } else {
                      clearingCharges += executionReport.m_lastQuantity *
                        CLEARING_FEE;
                    }
                  }
                }
              });
          }
        } catch(const PipeBrokenException&) {}
      }
      startDate += gregorian::days(1);
      if(!securities.empty()) {
        ++dayCount;
      }
    }
    if(dayCount == 0) {
      dayCount = 1;
    }
    cout << "Canadian Report\n";
    cout << "-------------------\n\n";
    cout << "Open Positions" << endl;
    for(const auto& accountPortfolio : accountPortfolios) {
      bool isEmpty = true;
      std::stringstream output;
      output << "\t " << accountPortfolio.first.m_name << endl;
      for(const auto& inventory :
          accountPortfolio.second.GetBookkeeper().GetInventoryRange()) {
        if(inventory.second.m_position.m_quantity != 0) {
          output << "\t\t " <<
            ToString(inventory.first.m_index, marketDatabase) << " " <<
            inventory.second.m_position.m_quantity << " " <<
            ToString(GetAveragePrice(inventory.second.m_position)) << endl;
          isEmpty = false;
        }
      }
      output << endl;
      if(!isEmpty) {
        cout << output.str();
      }
    }
    cout << endl;
    cout << "Destination Volume" << endl;
    for(auto i : destinationVolume) {
      cout << "\t" << i.first.GetData() << "\t" << i.second << endl;
    }
    cout << endl;
    auto totals = portfolio.GetBookkeeper().GetTotal(DefaultCurrencies::CAD());
    auto netProfitAndLoss = (1 - PAYOUT) * GetRealizedProfitAndLoss(totals);
    auto netFees = (orderCollections - ticketCharges) +
      (cdsCollections - cdsCharges) + (iirocCollections - iirocCharges) +
      (clearingCollections - clearingCharges) +
      (spireCollections - spireCharges);
    auto totalNet = netProfitAndLoss + netFees;
    cout << "Daily Averages" << endl;
    cout << "Ticket Fees        : " << (ticketCharges / dayCount).ToString() <<
      endl;
    cout << "CDS Charges        : " << (cdsCharges / dayCount).ToString() <<
      endl;
    cout << "CDS Collections    : " << (cdsCollections / dayCount).ToString() <<
      endl;
    cout << "IIROC Charges      : " << (iirocCharges / dayCount).ToString() <<
      endl;
    cout << "IIROC Collections  : " <<
      (iirocCollections / dayCount).ToString() << endl;
    cout << "Order Collections  : " <<
      (orderCollections / dayCount).ToString() << endl;
    cout << "Spire Fee          : " << (spireCharges / dayCount).ToString() <<
      endl;
    cout << "Spire Collections  : " <<
      (spireCollections / dayCount).ToString() << endl;
    cout << "Clearing Fee       : " <<
      (clearingCharges / dayCount).ToString() << endl;
    cout << "Clearing Collection: " <<
      (clearingCollections / dayCount).ToString() << endl;
    cout << "Execution Fee      : " <<
      (executionCharges / dayCount).ToString() << endl;
    cout << "Profit and Loss    : " <<
      (netProfitAndLoss / dayCount).ToString() << endl;
    cout << "Net Fees           : " << (netFees / dayCount).ToString() << endl;
    cout << "Total Net          : " << (totalNet / dayCount).ToString() << endl;
    cout << "Volume             : " << (totals.m_volume / dayCount) << endl;
    cout << "Notional Value     : " << (notionalValue / dayCount).ToString() <<
      endl;
    cout << "Trades             : " << (trades / dayCount) << endl;
    cout << endl << endl;
    cout << "Totals" << endl;
    cout << "Ticket Fees        : " << ticketCharges.ToString() << endl;
    cout << "CDS Charges        : " << cdsCharges.ToString() << endl;
    cout << "CDS Collections    : " << cdsCollections.ToString() << endl;
    cout << "IIROC Charges      : " << iirocCharges.ToString() << endl;
    cout << "IIROC Collections  : " << iirocCollections.ToString() << endl;
    cout << "Order Collections  : " << orderCollections.ToString() << endl;
    cout << "Spire Fee          : " << spireCharges.ToString() << endl;
    cout << "Spire Collections  : " << spireCollections.ToString() << endl;
    cout << "Clearing Fee       : " << clearingCharges.ToString() << endl;
    cout << "Clearing Collect   : " << clearingCollections.ToString() << endl;
    cout << "Execution Fee      : " << executionCharges.ToString() << endl;
    cout << "Profit and Loss    : " << netProfitAndLoss.ToString() << endl;
    cout << "Net Fees           : " << netFees.ToString() << endl;
    cout << "Total Net          : " << totalNet.ToString() << endl;
    cout << "Volume             : " << totals.m_volume << endl;
    cout << "Notional Value     : " << notionalValue.ToString() << endl;
    cout << "Trades             : " << trades << endl;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r\nCopyright (C) 2009 Eidolon Systems Ltd."};
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
  ptime startDate;
  ptime endDate;
  MarketDatabase marketDatabase;
  boost::local_time::tz_database timeZoneDatabase;
  vector<DirectoryEntry> accounts;
  try {
    startDate = Extract<ptime>(config, "start");
    endDate = Extract<ptime>(config, "end");
    marketDatabase = definitionsClient->LoadMarketDatabase();
    timeZoneDatabase = definitionsClient->LoadTimeZoneDatabase();
    accounts = serviceLocatorClient->LoadAllAccounts();
  } catch(const std::exception& e) {
    cerr << "Unable to initialize report: " << e.what() << endl;
    return -1;
  }
  ExecuteAustralianReport(startDate, endDate, accounts, marketDatabase,
    timeZoneDatabase, orderExecutionClient);
  cout << "\n\n\n\n";
  ExecuteCanadianReport(startDate, endDate, accounts, marketDatabase,
    timeZoneDatabase, orderExecutionClient);
  return 0;
}
