#include <deque>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/LocalValuationModel.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Spire;

namespace {
  auto TST = ParseSecurity("TST.NYSE");
}

TEST_SUITE("PortfolioModel") {
  TEST_CASE("construction") {
    run_test([] {
      auto orders = std::make_shared<ArrayListModel<const Order*>>();
      auto valuation = std::make_shared<LocalValuationModel>();
      auto tst_valuation =
        std::make_shared<CompositeModel<SecurityValuation>>();
      tst_valuation->add(&SecurityValuation::m_currency,
        make_constant_value_model(DefaultCurrencies::USD()));
      auto tst_bid = std::make_shared<LocalValueModel<optional<Money>>>();
      tst_valuation->add(&SecurityValuation::m_bidValue, tst_bid);
      auto tst_ask = std::make_shared<LocalValueModel<optional<Money>>>();
      tst_valuation->add(&SecurityValuation::m_askValue, tst_ask);
      valuation->add(TST, tst_valuation);
      auto portfolio =
        PortfolioModel(GetDefaultMarketDatabase(), {}, orders, valuation);
      auto updates = std::deque<PortfolioModel::Portfolio::UpdateEntry>();
      portfolio.connect_update_signal([&] (const auto& update) {
        updates.push_back(update);
      });
      auto fields =
        OrderFields::MakeLimitOrder(TST, Side::BID, 100, Money::ONE);
      fields.m_currency = DefaultCurrencies::USD();
      auto order =
        PrimitiveOrder(OrderInfo(fields, 1, second_clock::universal_time()));
      orders->push(&order);
      Fill(order, 100);
      FlushPendingRoutines();
      QApplication::processEvents();
      REQUIRE(updates.size() == 1);
      auto update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_securityInventory.m_position.m_key.m_index == TST);
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(
        update.m_securityInventory.m_position.m_costBasis == 100 * Money::ONE);
      REQUIRE(update.m_unrealizedSecurity == Money::ZERO);
      tst_bid->set(99 * Money::CENT);
      QApplication::processEvents();
      REQUIRE(updates.size() == 1);
      update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_unrealizedSecurity == -Money::ONE);
    });
  }
}
