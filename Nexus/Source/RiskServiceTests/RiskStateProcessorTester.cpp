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
  TEST_CASE("zero_net_loss") {
    auto timeClient = FixedTimeClient();
    timeClient.SetTime(ptime(date(2020, 3, 12), minutes(12)));
    timeClient.Open();
    auto parameters = RiskParameters(DefaultCurrencies::USD(), Money::ZERO,
      RiskState::Type::ACTIVE, Money::ZERO, 0, minutes(1));
    auto processor = RiskStateProcessor(
      RiskPortfolio(GetDefaultMarketDatabase()), parameters, {}, &timeClient);
    REQUIRE(processor.GetParameters() == parameters);
    REQUIRE(processor.GetRiskState() == RiskState::Type::ACTIVE);
    auto fields = OrderFields::BuildLimitOrder(TSLA, Side::BID, 100,
      Money::ONE);
    auto report = ExecutionReport::BuildInitialReport(1, timeClient.GetTime());
    processor.GetPortfolio().Update(fields, report);
    processor.Update();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    auto fillReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::FILLED, timeClient.GetTime());
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = Money::ONE;
    processor.GetPortfolio().Update(fields, report);
    processor.Update();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::ACTIVE);
    processor.GetPortfolio().UpdateBid(TSLA, 99 * Money::CENT);
    processor.Update();
    REQUIRE(processor.GetRiskState().m_type == RiskState::Type::CLOSE_ORDERS);
    REQUIRE(processor.GetRiskState() == RiskState(
      RiskState::Type::CLOSE_ORDERS, ptime(date(2020, 3, 12), minutes(13))));
  }
}
