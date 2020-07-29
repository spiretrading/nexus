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
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  using RiskPortfolio =
    Portfolio<TrueAverageBookkeeper<RiskPortfolioInventory>>;

  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
}

TEST_SUITE("RiskStateProcessor") {
  TEST_CASE("transition_closed_disabled") {
    auto timeClient = FixedTimeClient(ptime(date(2020, 3, 12), minutes(12)));
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(1));
    auto processor = RiskStateProcessor(
      RiskPortfolio(GetDefaultMarketDatabase()), parameters, {}, &timeClient);
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
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(13))));
    processor.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    timeClient.SetTime(ptime(date(2020, 3, 12), minutes(13)));
    processor.UpdateTime();
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(14))));
    timeClient.SetTime(ptime(date(2020, 3, 12), minutes(13) + seconds(30)));
    processor.UpdateTime();
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(14))));
    timeClient.SetTime(ptime(date(2020, 3, 12), minutes(14)));
    processor.UpdateTime();
    REQUIRE(processor.GetRiskState() == RiskState::Type::DISABLED);
    processor.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    processor.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    processor.UpdatePortfolio();
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(15))));
  }

  TEST_CASE("immediate_close") {
    auto timeClient = FixedTimeClient(ptime(date(2020, 3, 12), minutes(12)));
    auto portfolio = RiskPortfolio(GetDefaultMarketDatabase());
    portfolio.Update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields = OrderFields::BuildLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::BuildInitialReport(1, timeClient.GetTime());
    portfolio.Update(fields, report);
    auto fillReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    portfolio.Update(fields, fillReport);
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(1));
    auto processor = RiskStateProcessor(portfolio, parameters, {}, &timeClient);
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(13))));
  }
}
