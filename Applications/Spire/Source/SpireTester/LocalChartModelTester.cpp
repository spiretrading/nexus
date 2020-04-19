#include <doctest/doctest.h>
#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/SpireTester/TestChartModel.hpp"

using namespace Beam::Queries;
using namespace Nexus;
using namespace Spire;

namespace {
  auto create_model() {
    auto local_model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
      Scalar::Type::MONEY,
      [=] {
        auto candlesticks = std::vector<Candlestick>();
        for(auto i = 0; i < 101; ++i) {
          candlesticks.push_back({Scalar(i * Money::ONE),
            Scalar((i + 1) * Money::ONE)});
        }
        return candlesticks;
      }());
    return local_model;
  }

  std::vector<Candlestick> load(int first, int last,
      const SnapshotLimit& limit) {
    auto model = create_model();
    return std::move(wait(model->load(Scalar(first * Money::ONE),
      Scalar(last * Money::ONE), limit)));
  }

  std::vector<Candlestick> load(LocalChartModel* model, int first, int last,
      const SnapshotLimit& limit) {
    return std::move(wait(model->load(Scalar(first * Money::ONE),
      Scalar(last * Money::ONE), limit)));
  }

  std::vector<Candlestick> generate_range(int first, int last) {
    auto candlesticks = std::vector<Candlestick>();
    for(auto i = first; i < last; ++i) {
      candlesticks.push_back({Scalar(i), Scalar(i + 1)});
    }
    return candlesticks;
  }

  Candlestick make(int start, int end) {
    return {Scalar(start), Scalar(end)};
  }

  auto create_coincident_model() {
    auto model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
      Scalar::Type::MONEY,
      [=] {
        auto candlesticks = std::vector<Candlestick>();
        for(auto i = 0; i < 101; ++i) {
          auto candlestick = Candlestick(Scalar(i * Money::ONE),
            Scalar((i + 1) * Money::ONE));
          candlesticks.insert(candlesticks.end(), 5, candlestick);
        }
        return candlesticks;
      }());
    return model;
  }
}

TEST_SUITE("LocalChartModel") {
  TEST_CASE("unlimited_loads") {
    run_test([=] {
      auto load1 = load(0, 100, SnapshotLimit::Unlimited());
      REQUIRE(load1.size() == 101);
      REQUIRE(load1.front().GetStart() == Scalar(0));
      REQUIRE(load1.back().GetEnd() == Scalar(101));
      auto load2 = load(25, 75, SnapshotLimit::Unlimited());
      REQUIRE(load2.size() == 52);
      REQUIRE(load2.front().GetStart() == Scalar(24));
      REQUIRE(load2.back().GetEnd() == Scalar(76));
      auto load3 = load(-10, 25, SnapshotLimit::Unlimited());
      REQUIRE(load3.size() == 26);
      REQUIRE(load3.front().GetStart() == Scalar(0));
      REQUIRE(load3.back().GetEnd() == Scalar(26));
      auto load4 = load(75, 110, SnapshotLimit::Unlimited());
      REQUIRE(load4.size() == 27);
      REQUIRE(load4.front().GetStart() == Scalar(74));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
    }, "unlimited_loads");
  }

  TEST_CASE("loads_from_head") {
    run_test([=] {
      auto load1 = load(0, 100, SnapshotLimit::FromHead(25));
      REQUIRE(load1.size() == 25);
      REQUIRE(load1.front().GetStart() == Scalar(0));
      REQUIRE(load1.back().GetEnd() == Scalar(25));
      auto load2 = load(50, 100, SnapshotLimit::FromHead(25));
      REQUIRE(load2.size() == 25);
      REQUIRE(load2.front().GetStart() == Scalar(49));
      REQUIRE(load2.back().GetEnd() == Scalar(74));
      auto load3 = load(-10, 25, SnapshotLimit::FromHead(10));
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(0));
      REQUIRE(load3.back().GetEnd() == Scalar(10));
      auto load4 = load(95, 110, SnapshotLimit::FromHead(10));
      REQUIRE(load4.size() == 7);
      REQUIRE(load4.front().GetStart() == Scalar(94));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
    }, "loads_from_head");
  }

  TEST_CASE("loads_from_tail") {
    run_test([=] {
      auto load1 = load(0, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load1.size() == 25);
      REQUIRE(load1.front().GetStart() == Scalar(76));
      REQUIRE(load1.back().GetEnd() == Scalar(101));
      auto load2 = load(50, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load2.size() == 25);
      REQUIRE(load2.front().GetStart() == Scalar(76));
      REQUIRE(load2.back().GetEnd() == Scalar(101));
      auto load3 = load(-10, 25, SnapshotLimit::FromTail(10));
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(16));
      REQUIRE(load3.back().GetEnd() == Scalar(26));
      auto load4 = load(95, 110, SnapshotLimit::FromTail(10));
      REQUIRE(load4.size() == 7);
      REQUIRE(load4.front().GetStart() == Scalar(94));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
    }, "loads_from_tail");
  }

  TEST_CASE("storing_and_loading") {
    run_test([=] {
      auto model = LocalChartModel(Scalar::Type::MONEY, Scalar::Type::MONEY,
        {});
      auto load1 = load(&model, 0, 100, SnapshotLimit::Unlimited());
      REQUIRE(load1.size() == 0);
      model.store(generate_range(10, 20));
      auto load2 = load(&model, 0, 25, SnapshotLimit::Unlimited());
      REQUIRE(load2.size() == 10);
      REQUIRE(load2.front().GetStart() == Scalar(10));
      REQUIRE(load2.back().GetEnd() == Scalar(20));
      model.store(generate_range(30, 40));
      auto load3 = load(&model, 25, 50, SnapshotLimit::Unlimited());
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(30));
      REQUIRE(load3.back().GetEnd() == Scalar(40));
      auto load4 = load(&model, 0, 100, SnapshotLimit::Unlimited());
      REQUIRE(load4.size() == 20);
      REQUIRE(load4.front().GetStart() == Scalar(10));
      REQUIRE(load4.back().GetEnd() == Scalar(40));
    }, "storing_and_loading");
  }

  TEST_CASE("merging_data") {
    run_test([=] {
      auto model1 = LocalChartModel(Scalar::Type::MONEY, Scalar::Type::MONEY,
        {});
      model1.store({make(0, 1), make(2, 3), make(4, 5), make(6, 7),
        make(8, 9)});
      auto load1 = load(&model1, 0, 10, SnapshotLimit::Unlimited());
      REQUIRE(load1.size() == 5);
      model1.store({make(1, 2), make(3, 4), make(5, 6), make(7, 8),
        make(9, 10)});
      auto load2 = load(&model1, 0, 10, SnapshotLimit::Unlimited());
      REQUIRE(load2 == generate_range(0, 10));
      auto model2 = LocalChartModel(Scalar::Type::MONEY, Scalar::Type::MONEY,
        {});
      model2.store({make(4, 5), make(5, 6), make(6, 7), make(7, 8),
        make(8, 9)});
      model2.store({make(1, 2), make(2, 3), make(3, 4)});
      auto load3 = load(&model2, 1, 9, SnapshotLimit::Unlimited());
      REQUIRE(load3 == generate_range(1, 9));
    }, "merging_data");
  }

  TEST_CASE("coincidental_values") {
    run_test([=] {
      auto model = create_coincident_model();
      auto model_sticks = load(model.get(), 40, 40, SnapshotLimit::Unlimited());
      auto result = std::vector<Candlestick>({make(39, 40), make(39, 40),
        make(39, 40), make(39, 40), make(39, 40), make(40, 41), make(40, 41),
        make(40, 41), make(40, 41), make(40, 41)});
      REQUIRE(model_sticks == result);
    }, "coincidental_values");
  }
}
