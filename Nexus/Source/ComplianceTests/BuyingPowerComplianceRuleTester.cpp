#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto TST =
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());

  struct Fixture {
    TestEnvironment m_testEnvironment;
    std::shared_ptr<Beam::Queue<const Order*>> m_orderSubmissions;
    TestServiceClients m_serviceClients;

    Fixture()
        : m_orderSubmissions(std::make_shared<Queue<const Order*>>()),
          m_serviceClients(Ref(m_testEnvironment)) {
      m_testEnvironment.MonitorOrderSubmissions(m_orderSubmissions);
      m_testEnvironment.UpdateBboPrice(
        TST, Money::ONE, Money::ONE + Money::CENT);
    }
  };
}

TEST_SUITE("BuyingPowerComplianceRule") {
  TEST_CASE_FIXTURE(Fixture, "order_recovery") {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("currency", DefaultCurrencies::USD());
    parameters.emplace_back("buying_power", 1000 * Money::ONE);
    auto symbols = std::vector<ComplianceValue>();
    symbols.push_back(Security("*", "", CountryCode::NONE));
    parameters.emplace_back("symbols", symbols);
    auto rule = BuyingPowerComplianceRule(
      parameters, {}, &m_serviceClients.GetMarketDataClient());
    auto recoveryFields =
      OrderFields::MakeLimitOrder(TST, Side::BID, 100, Money::ONE);
    auto& recoverOrder =
      m_serviceClients.GetOrderExecutionClient().Submit(recoveryFields);
    auto submittedRecoveryOrder = m_orderSubmissions->Pop();
    m_testEnvironment.Accept(*submittedRecoveryOrder);
    m_testEnvironment.Fill(*submittedRecoveryOrder, 100);
    REQUIRE_NOTHROW(rule.Add(*submittedRecoveryOrder));
    auto submissionFields =
      OrderFields::MakeLimitOrder(TST, Side::BID, 100, 2 * Money::ONE);
    auto& submissionOrder =
      m_serviceClients.GetOrderExecutionClient().Submit(submissionFields);
    auto submittedOrder = m_orderSubmissions->Pop();
    REQUIRE_NOTHROW(rule.Submit(*submittedOrder));
  }
}
