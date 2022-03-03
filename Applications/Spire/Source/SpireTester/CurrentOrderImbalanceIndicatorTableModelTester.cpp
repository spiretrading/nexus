#include <doctest/doctest.h>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::TimeService::Tests;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_imbalance(const auto& symbol, auto timestamp) {
    return OrderImbalance(Security(symbol, DefaultCountries::US()), Side::ASK,
      Quantity(timestamp), Money(timestamp), from_time_t(timestamp));
  }

  auto row_equals(const auto& model, auto row, auto imbalance) {
    return model.get<Security>(row, 0) == imbalance.m_security &&
      model.get<Side>(row, 1) == imbalance.m_side &&
      model.get<Quantity>(row, 2) == imbalance.m_size &&
      model.get<Money>(row, 3) == imbalance.m_referencePrice &&
      model.get<Money>(row, 4) ==
        imbalance.m_size * imbalance.m_referencePrice &&
      model.get<date>(row, 5) == imbalance.m_timestamp.date() &&
      model.get<time_duration>(row, 6) == imbalance.m_timestamp.time_of_day();
  }

  auto rows_equal(
      const auto& model, const auto& imbalance1, const auto& imbalance2) {
    if(model.get_row_size() != 2) {
      return false;
    }
    return
      row_equals(model, 0, imbalance1) && row_equals(model, 1, imbalance2) ||
      row_equals(model, 0, imbalance2) && row_equals(model, 1, imbalance1);
  }

  template<typename... F>
  decltype(auto) test_operation(
      const TableModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }

  void wait_until(const std::function<bool ()>& predicate) {
    while(!predicate()) {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      QCoreApplication::sendPostedEvents();
    }
  }

  auto A100 = make_imbalance("A", 100);
  auto A300 = make_imbalance("A", 300);
  auto B150 = make_imbalance("B", 150);
  auto B350 = make_imbalance("B", 350);
}

TEST_SUITE("CurrentOrderImbalanceIndicatorTableModel") {
  TEST_CASE("offset") {
    auto environment = TimeServiceTestEnvironment(from_time_t(0));
    auto timer_factory = CurrentOrderImbalanceIndicatorTableModel::TimerFactory(
      [&] (auto duration) {
        // TODO: TestTimer type, ptr or ?
        return TimerBox(std::make_unique<TestTimer>(duration, Ref(environment)));
      });
    auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    auto model = std::make_unique<CurrentOrderImbalanceIndicatorTableModel>(
      time_duration(seconds(200)),
      // TODO: TestTimeClient type, ptr or ?
      TimeClientBox(std::make_unique<TestTimeClient>(Ref(environment))),
      timer_factory, source);
    //source->publish(A100);
    //wait_until([&] { return model.get_row_size() == 1; });
    //REQUIRE(model.get_row_size() == 1);
    //REQUIRE(row_equals(model, 0, A100));
    //source->publish(B150);
    //wait_until([&] { return model.get_row_size() == 2; });
    //REQUIRE(model.get_row_size() == 2);
    //REQUIRE(rows_equal(model, A100, B150));
    //environment.AdvanceTime(seconds(100));
    //REQUIRE(model.get_row_size() == 1);
    //REQUIRE(row_equals(model, 0, B150));
    model.reset();
  }

  TEST_CASE("operations") {
    
  }
}
