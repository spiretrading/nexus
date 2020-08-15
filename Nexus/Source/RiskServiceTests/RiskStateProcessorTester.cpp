#include <doctest/doctest.h>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskStateProcessor.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());
}

TEST_SUITE("RiskStateProcessor") {
  TEST_CASE("transition_closed_disabled") {
    auto timeClient = FixedTimeClient(time_from_string("2020-03-20 13:12:00"));
    auto processor = RiskStateProcessor(GetDefaultMarketDatabase(), {},
      &timeClient);
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(1));
    processor.Update(parameters);
    REQUIRE(processor.GetParameters() == parameters);
    REQUIRE(processor.GetRiskState() == RiskState::Type::ACTIVE);
    auto fields = OrderFields::BuildLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::BuildInitialReport(1, timeClient.GetTime());
    processor.GetPortfolio().Update(fields, report);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    auto fillReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    processor.GetPortfolio().Update(fields, fillReport);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:13:00")));
    processor.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    timeClient.SetTime(time_from_string("2020-03-20 13:13:00"));
    processor.UpdateTime();
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:14:00")));
    timeClient.SetTime(time_from_string("2020-03-20 13:13:30"));
    processor.UpdateTime();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:14:00")));
    timeClient.SetTime(time_from_string("2020-03-20 13:14:00"));
    processor.UpdateTime();
    REQUIRE(processor.GetRiskState() == RiskState::Type::DISABLED);
    processor.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:15:00")));
  }

  TEST_CASE("immediate_close") {
    auto timeClient = FixedTimeClient(time_from_string("1995-01-22 14:16:00"));
    auto processor = RiskStateProcessor(GetDefaultMarketDatabase(), {},
      &timeClient);
    processor.Update(RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(5)));
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields = OrderFields::BuildLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::BuildInitialReport(1, timeClient.GetTime());
    processor.GetPortfolio().Update(fields, report);
    auto fillReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    processor.GetPortfolio().Update(fields, fillReport);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("1995-01-22 14:21:00")));
  }

  TEST_CASE("mixed_currencies") {
    auto timeClient = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto exchangeRates = std::vector<ExchangeRate>();
    exchangeRates.push_back(ExchangeRate(ParseCurrencyPair("USD/CAD"),
      rational<int>(1, 2)));
    auto processor = RiskStateProcessor(GetDefaultMarketDatabase(),
      exchangeRates, &timeClient);
    processor.Update(RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, 10 * Money::ONE, 0, minutes(2)));
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.GetPortfolio().Update(XIU, 2 * Money::ONE,
      Money::ONE + 99 * Money::CENT);
    auto tslaFields = OrderFields::BuildLimitOrder(TSLA,
      DefaultCurrencies::USD(), Side::BID, 100, Money::ONE);
    auto tslaReport = ExecutionReport::BuildInitialReport(1,
      timeClient.GetTime());
    processor.GetPortfolio().Update(tslaFields, tslaReport);
    processor.UpdatePortfolio();
    auto tslaFillReport = ExecutionReport::BuildUpdatedReport(tslaReport,
      OrderStatus::FILLED, timeClient.GetTime());
    tslaFillReport.m_lastQuantity = 100;
    tslaFillReport.m_lastPrice = Money::ONE;
    processor.GetPortfolio().Update(tslaFields, tslaFillReport);
    processor.UpdatePortfolio();
    auto xiuFields = OrderFields::BuildLimitOrder(XIU, DefaultCurrencies::CAD(),
      Side::ASK, 100, 2 * Money::ONE);
    auto xiuReport = ExecutionReport::BuildInitialReport(2,
      timeClient.GetTime());
    processor.GetPortfolio().Update(xiuFields, xiuReport);
    processor.UpdatePortfolio();
    auto xiuFillReport = ExecutionReport::BuildUpdatedReport(xiuReport,
      OrderStatus::FILLED, timeClient.GetTime());
    xiuFillReport.m_lastQuantity = 100;
    xiuFillReport.m_lastPrice = 2 * Money::ONE;
    processor.GetPortfolio().Update(xiuFields, xiuFillReport);
    processor.UpdatePortfolio();
    processor.GetPortfolio().Update(XIU, 2 * Money::ONE + 4 * Money::CENT,
      2 * Money::ONE + 5 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState::Type::ACTIVE);
    processor.GetPortfolio().Update(XIU, 2 * Money::ONE + 5 * Money::CENT,
      2 * Money::ONE + 6 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2006-07-2 3:13:30")));
  }

  TEST_CASE("unknown_currency") {
    auto timeClient = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto processor = RiskStateProcessor(GetDefaultMarketDatabase(), {},
      &timeClient);
    processor.Update(RiskParameters(DefaultCurrencies::JPY(), Money::ZERO,
      RiskState::Type::ACTIVE, 10 * Money::ONE, 0, minutes(2)));
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields = OrderFields::BuildLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::BuildInitialReport(1, timeClient.GetTime());
    processor.GetPortfolio().Update(fields, report);
    processor.UpdatePortfolio();
    auto fillReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    processor.GetPortfolio().Update(fields, fillReport);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2006-07-2 3:13:30")));
  }
}
