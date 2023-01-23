#include <doctest/doctest.h>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskStateModel.hpp"

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

TEST_SUITE("RiskStateModel") {
  TEST_CASE("transition_closed_disabled") {
    auto timeClient = FixedTimeClient(time_from_string("2020-03-20 13:12:00"));
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(1));
    auto model = RiskStateModel(RiskPortfolio(GetDefaultMarketDatabase()),
      parameters, {}, &timeClient);
    REQUIRE(model.GetParameters() == parameters);
    REQUIRE(model.GetRiskState() == RiskState::Type::ACTIVE);
    auto fields = OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::MakeInitialReport(1, timeClient.GetTime());
    model.GetPortfolio().Update(fields, report);
    model.UpdatePortfolio();
    REQUIRE((model.GetRiskState().m_type == RiskState::Type::ACTIVE));
    auto fillReport = ExecutionReport::MakeUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    model.GetPortfolio().Update(fields, fillReport);
    model.UpdatePortfolio();
    REQUIRE((model.GetRiskState().m_type == RiskState::Type::ACTIVE));
    model.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:13:00")));
    model.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    model.UpdatePortfolio();
    REQUIRE((model.GetRiskState().m_type == RiskState::Type::ACTIVE));
    timeClient.SetTime(time_from_string("2020-03-20 13:13:00"));
    model.UpdateTime();
    model.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:14:00")));
    timeClient.SetTime(time_from_string("2020-03-20 13:13:30"));
    model.UpdateTime();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:14:00")));
    timeClient.SetTime(time_from_string("2020-03-20 13:14:00"));
    model.UpdateTime();
    REQUIRE(model.GetRiskState() == RiskState::Type::DISABLED);
    model.GetPortfolio().Update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    model.UpdatePortfolio();
    REQUIRE((model.GetRiskState().m_type == RiskState::Type::ACTIVE));
    model.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2020-03-20 13:15:00")));
  }

  TEST_CASE("immediate_close") {
    auto timeClient = FixedTimeClient(time_from_string("1995-01-22 14:16:00"));
    auto portfolio = RiskPortfolio(GetDefaultMarketDatabase());
    portfolio.Update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields = OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::MakeInitialReport(1, timeClient.GetTime());
    portfolio.Update(fields, report);
    auto fillReport = ExecutionReport::MakeUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    portfolio.Update(fields, fillReport);
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ONE, 0, minutes(5));
    auto model = RiskStateModel(portfolio, parameters, {}, &timeClient);
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("1995-01-22 14:21:00")));
  }

  TEST_CASE("mixed_currencies") {
    auto timeClient = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, 10 * Money::ONE, 0, minutes(2));
    auto exchangeRates = std::vector<ExchangeRate>();
    exchangeRates.push_back(ExchangeRate(ParseCurrencyPair("USD/CAD"),
      rational<int>(1, 2)));
    auto model = RiskStateModel(RiskPortfolio(GetDefaultMarketDatabase()),
      parameters, exchangeRates, &timeClient);
    model.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    model.GetPortfolio().Update(XIU, 2 * Money::ONE,
      Money::ONE + 99 * Money::CENT);
    auto tslaFields = OrderFields::MakeLimitOrder(TSLA,
      DefaultCurrencies::USD(), Side::BID, 100, Money::ONE);
    auto tslaReport = ExecutionReport::MakeInitialReport(1,
      timeClient.GetTime());
    model.GetPortfolio().Update(tslaFields, tslaReport);
    model.UpdatePortfolio();
    auto tslaFillReport = ExecutionReport::MakeUpdatedReport(tslaReport,
      OrderStatus::FILLED, timeClient.GetTime());
    tslaFillReport.m_lastQuantity = 100;
    tslaFillReport.m_lastPrice = Money::ONE;
    model.GetPortfolio().Update(tslaFields, tslaFillReport);
    model.UpdatePortfolio();
    auto xiuFields = OrderFields::MakeLimitOrder(XIU, DefaultCurrencies::CAD(),
      Side::ASK, 100, 2 * Money::ONE);
    auto xiuReport = ExecutionReport::MakeInitialReport(2,
      timeClient.GetTime());
    model.GetPortfolio().Update(xiuFields, xiuReport);
    model.UpdatePortfolio();
    auto xiuFillReport = ExecutionReport::MakeUpdatedReport(xiuReport,
      OrderStatus::FILLED, timeClient.GetTime());
    xiuFillReport.m_lastQuantity = 100;
    xiuFillReport.m_lastPrice = 2 * Money::ONE;
    model.GetPortfolio().Update(xiuFields, xiuFillReport);
    model.UpdatePortfolio();
    model.GetPortfolio().Update(XIU, 2 * Money::ONE + 4 * Money::CENT,
      2 * Money::ONE + 5 * Money::CENT);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState::Type::ACTIVE);
    model.GetPortfolio().Update(XIU, 2 * Money::ONE + 5 * Money::CENT,
      2 * Money::ONE + 6 * Money::CENT);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2006-07-2 3:13:30")));
  }

  TEST_CASE("unknown_currency") {
    auto timeClient = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto parameters = RiskParameters(DefaultCurrencies::JPY(), Money::ZERO,
      RiskState::Type::ACTIVE, 10 * Money::ONE, 0, minutes(2));
    auto model = RiskStateModel(RiskPortfolio(GetDefaultMarketDatabase()),
      parameters, {}, &timeClient);
    model.GetPortfolio().Update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields = OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE);
    auto report = ExecutionReport::MakeInitialReport(1, timeClient.GetTime());
    model.GetPortfolio().Update(fields, report);
    model.UpdatePortfolio();
    auto fillReport = ExecutionReport::MakeUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    model.GetPortfolio().Update(fields, fillReport);
    model.UpdatePortfolio();
    REQUIRE(model.GetRiskState() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2006-07-2 3:13:30")));
  }
}
