#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/Evaluator.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  auto make_ticker_info() {
    auto info = TickerInfo();
    info.m_name = "Test";
    info.m_tick_size = Money::CENT;
    info.m_lot_size = 100;
    info.m_board_lot = 100;
    info.m_price_precision = 2;
    info.m_quantity_precision = 0;
    info.m_multiplier = Quantity(1);
    return info;
  }

  auto make_ticker_info_expression() {
    return ConstantExpression(make_ticker_info());
  }
}

TEST_SUITE("EvaluatorTranslator") {
  TEST_CASE("translate_ticker_info_name") {
    auto expression = MemberAccessExpression(
      "name", typeid(std::string), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_instrument") {
    auto expression = MemberAccessExpression(
      "instrument", typeid(Instrument), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_tick_size") {
    auto expression = MemberAccessExpression(
      "tick_size", typeid(Money), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_lot_size") {
    auto expression = MemberAccessExpression(
      "lot_size", typeid(Quantity), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_board_lot") {
    auto expression = MemberAccessExpression(
      "board_lot", typeid(Quantity), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_price_precision") {
    auto expression = MemberAccessExpression(
      "price_precision", typeid(int), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_quantity_precision") {
    auto expression = MemberAccessExpression(
      "quantity_precision", typeid(int), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }

  TEST_CASE("translate_ticker_info_multiplier") {
    auto expression = MemberAccessExpression(
      "multiplier", typeid(Quantity), make_ticker_info_expression());
    auto translator = Nexus::EvaluatorTranslator();
    translator.translate(expression);
  }
}
