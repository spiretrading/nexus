#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskStateModel.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  auto TSLA = Security("TSLA", NASDAQ);
  auto XIU = Security("XIU", TSX);
}

TEST_SUITE("RiskStateModel") {
  TEST_CASE("constructor") {
    auto time_client = FixedTimeClient(time_from_string("2022-01-01 10:00:00"));
    auto portfolio = RiskPortfolio(DEFAULT_VENUES);
    auto allowed_state = RiskState(RiskState::Type::ACTIVE);
    auto parameters =
      RiskParameters(USD, Money::ZERO, allowed_state, Money::ONE, minutes(1));
    auto model =
      RiskStateModel(portfolio, parameters, ExchangeRateTable(), &time_client);
    REQUIRE(model.get_portfolio().get_security_entries().size() == 0);
    REQUIRE(model.get_parameters() == parameters);
    REQUIRE(model.get_risk_state() == allowed_state);
    auto new_allowed_state = RiskState(RiskState::Type::CLOSE_ORDERS);
    auto new_parameters = RiskParameters(
      USD, Money::ZERO, new_allowed_state, Money::ONE, minutes(1));
    model.update(new_parameters);
    REQUIRE(model.get_parameters() == new_parameters);
    REQUIRE(model.get_risk_state() == new_allowed_state);
  }

  TEST_CASE("transition_closed_disabled") {
    auto time_client = FixedTimeClient(time_from_string("2020-03-20 13:12:00"));
    auto parameters = RiskParameters(
      USD, Money::ZERO, RiskState::Type::ACTIVE, Money::ONE, minutes(1));
    auto model = RiskStateModel(RiskPortfolio(DEFAULT_VENUES), parameters,
      ExchangeRateTable(), &time_client);
    REQUIRE(model.get_parameters() == parameters);
    REQUIRE(model.get_risk_state() == RiskState::Type::ACTIVE);
    auto fields =
      make_limit_order_fields(TSLA, USD, Side::BID, 100, Money::ONE);
    auto report = ExecutionReport(1, time_client.GetTime());
    model.get_portfolio().update(fields, report);
    model.update_portfolio();
    REQUIRE(model.get_risk_state().m_type == RiskState::Type::ACTIVE);
    auto fill_report =
      make_update(report, OrderStatus::FILLED, time_client.GetTime());
    fill_report.m_last_quantity = 100;
    fill_report.m_last_price = Money::ONE;
    model.get_portfolio().update(fields, fill_report);
    model.update_portfolio();
    REQUIRE(model.get_risk_state().m_type == RiskState::Type::ACTIVE);
    model.get_portfolio().update(TSLA, Money::ONE, 99 * Money::CENT);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2020-03-20 13:13:00")));
    model.get_portfolio().update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    model.update_portfolio();
    REQUIRE(model.get_risk_state().m_type == RiskState::Type::ACTIVE);
    time_client.SetTime(time_from_string("2020-03-20 13:13:00"));
    model.update_time();
    model.get_portfolio().update(TSLA, Money::ONE, 99 * Money::CENT);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2020-03-20 13:14:00")));
    time_client.SetTime(time_from_string("2020-03-20 13:13:30"));
    model.update_time();
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2020-03-20 13:14:00")));
    time_client.SetTime(time_from_string("2020-03-20 13:14:00"));
    model.update_time();
    REQUIRE(model.get_risk_state() == RiskState::Type::DISABLED);
    model.get_portfolio().update(TSLA, Money::ONE + Money::CENT, Money::ONE);
    model.update_portfolio();
    REQUIRE(model.get_risk_state().m_type == RiskState::Type::ACTIVE);
    model.get_portfolio().update(TSLA, Money::ONE, 99 * Money::CENT);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2020-03-20 13:15:00")));
  }

  TEST_CASE("immediate_close") {
    auto time_client = FixedTimeClient(time_from_string("1995-01-22 14:16:00"));
    auto portfolio = RiskPortfolio(DEFAULT_VENUES);
    portfolio.update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields =
      make_limit_order_fields(TSLA, USD, Side::BID, 100, Money::ONE);
    auto report = ExecutionReport(1, time_client.GetTime());
    portfolio.update(fields, report);
    auto fill_report =
      make_update(report, OrderStatus::FILLED, time_client.GetTime());
    fill_report.m_last_quantity = 100;
    fill_report.m_last_price = Money::ONE;
    portfolio.update(fields, fill_report);
    auto parameters = RiskParameters(
      USD, Money::ZERO, RiskState::Type::ACTIVE, Money::ONE, minutes(5));
    auto model =
      RiskStateModel(portfolio, parameters, ExchangeRateTable(), &time_client);
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("1995-01-22 14:21:00")));
  }

  TEST_CASE("mixed_currencies") {
    auto time_client = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto parameters = RiskParameters(
      USD, Money::ZERO, RiskState::Type::ACTIVE, 10 * Money::ONE, minutes(2));
    auto exchange_rates = ExchangeRateTable();
    exchange_rates.add(
      ExchangeRate(parse_currency_pair("USD/CAD"), rational<int>(1, 2)));
    auto model = RiskStateModel(
      RiskPortfolio(DEFAULT_VENUES), parameters, exchange_rates, &time_client);
    model.get_portfolio().update(TSLA, Money::ONE, 99 * Money::CENT);
    model.get_portfolio().update(
      XIU, 2 * Money::ONE, Money::ONE + 99 * Money::CENT);
    auto tsla_fields =
      make_limit_order_fields(TSLA, USD, Side::BID, 100, Money::ONE);
    auto tsla_report = ExecutionReport(1, time_client.GetTime());
    model.get_portfolio().update(tsla_fields, tsla_report);
    model.update_portfolio();
    auto tsla_fill_report =
      make_update(tsla_report, OrderStatus::FILLED, time_client.GetTime());
    tsla_fill_report.m_last_quantity = 100;
    tsla_fill_report.m_last_price = Money::ONE;
    model.get_portfolio().update(tsla_fields, tsla_fill_report);
    model.update_portfolio();
    auto xiu_fields =
      make_limit_order_fields(XIU, CAD, Side::ASK, 100, 2 * Money::ONE);
    auto xiu_report = ExecutionReport(2, time_client.GetTime());
    model.get_portfolio().update(xiu_fields, xiu_report);
    model.update_portfolio();
    auto xiu_fill_report =
      make_update(xiu_report, OrderStatus::FILLED, time_client.GetTime());
    xiu_fill_report.m_last_quantity = 100;
    xiu_fill_report.m_last_price = 2 * Money::ONE;
    model.get_portfolio().update(xiu_fields, xiu_fill_report);
    model.update_portfolio();
    model.get_portfolio().update(
      XIU, 2 * Money::ONE + 4 * Money::CENT, 2 * Money::ONE + 5 * Money::CENT);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState::Type::ACTIVE);
    model.get_portfolio().update(
      XIU, 2 * Money::ONE + 5 * Money::CENT, 2 * Money::ONE + 6 * Money::CENT);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState(RiskState::Type::CLOSE_ORDERS,
      time_from_string("2006-07-2 3:13:30")));
  }

  TEST_CASE("unknown_currency") {
    auto time_client = FixedTimeClient(time_from_string("2006-07-2 3:11:30"));
    auto parameters = RiskParameters(
      JPY, Money::ZERO, RiskState::Type::ACTIVE, 10 * Money::ONE, minutes(2));
    auto model = RiskStateModel(RiskPortfolio(DEFAULT_VENUES), parameters,
      ExchangeRateTable(), &time_client);
    model.get_portfolio().update(TSLA, Money::ONE, 99 * Money::CENT);
    auto fields =
      make_limit_order_fields(TSLA, USD, Side::BID, 100, Money::ONE);
    auto report = ExecutionReport(1, time_client.GetTime());
    model.get_portfolio().update(fields, report);
    model.update_portfolio();
    auto fill_report =
      make_update(report, OrderStatus::FILLED, time_client.GetTime());
    fill_report.m_last_quantity = 100;
    fill_report.m_last_price = Money::ONE;
    model.get_portfolio().update(fields, fill_report);
    model.update_portfolio();
    REQUIRE(model.get_risk_state() == RiskState(
      RiskState::Type::CLOSE_ORDERS, time_from_string("2006-07-2 3:13:30")));
  }
}
