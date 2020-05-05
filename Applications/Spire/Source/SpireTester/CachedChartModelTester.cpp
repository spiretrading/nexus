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

  auto load(ChartModel* model, int first, int last,
      const SnapshotLimit& limit) {
    return wait(model->load(Scalar(first * Money::ONE),
      Scalar(last * Money::ONE), limit));
  }

  auto make(int start, int end) {
    return Candlestick(Scalar(start), Scalar(end));
  }
}

TEST_SUITE("CachedChartModel") {
  TEST_CASE("right_no_overlap") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 10, 20, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 10, 20, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 30, 40, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 30, 40, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("left_no_overlap") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 20, 30, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 20, 30, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 10, 18, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 10, 18, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("right_overlap") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 25, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("left_overlap") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 25, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 10, 30, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 10, 30, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("subset") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 25, 75, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 75, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("single_superset") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 20, 80, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("multiple_supersets") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 20, 80, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      model_sticks = load(model.get(), 10, 90, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 10, 90, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("multiple_subsets_and_supersets") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 40, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(model.get(), 60, 70, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 60, 70, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      model_sticks = load(model.get(), 80, 90, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 80, 90, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      model_sticks = load(model.get(), 45, 72, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 45, 72, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      model_sticks = load(model.get(), 35, 95, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 35, 95, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      model_sticks = load(model.get(), 40, 60, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("multiple_cache_hits") {
    run_test([] {
      auto model = create_model();
      auto test_model = TestChartModel(model->get_x_axis_type(),
        model->get_y_axis_type());
      auto cache = CachedChartModel(test_model);
      auto pre_load1 = cache.load(Scalar(40 * Money::ONE),
        Scalar(50 * Money::ONE), SnapshotLimit::Unlimited());
      wait(test_model.pop_load())->set_result(wait(
        model->load(Scalar(40 * Money::ONE), Scalar(50 * Money::ONE),
        SnapshotLimit::Unlimited())));
      wait(std::move(pre_load1));
      auto pre_load2 = cache.load(Scalar(60 * Money::ONE),
        Scalar(70 * Money::ONE), SnapshotLimit::Unlimited());
      wait(test_model.pop_load())->set_result(wait(
        model->load(Scalar(60 * Money::ONE), Scalar(70 * Money::ONE),
        SnapshotLimit::Unlimited())));
      wait(std::move(pre_load2));
      cache.load(Scalar(45 * Money::ONE), Scalar(75 * Money::ONE),
        SnapshotLimit::Unlimited());
      auto cache_load1 = wait(test_model.pop_load());
      REQUIRE(cache_load1->get_first() == Scalar(50 * Money::ONE));
      REQUIRE(cache_load1->get_last() == Scalar(60 * Money::ONE));
      cache_load1->set_result({});
      auto cache_load2 = wait(test_model.pop_load());
      REQUIRE(cache_load2->get_first() == Scalar(70 * Money::ONE));
      REQUIRE(cache_load2->get_last() == Scalar(75 * Money::ONE));
      cache_load2->set_result({});
    });
  }

  TEST_CASE("cached_model_loads_from_head") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      model->load(Scalar(0), Scalar(100), SnapshotLimit::Unlimited());
      auto load1 = load(&cache, 0, 100, SnapshotLimit::FromHead(25));
      REQUIRE(load1.size() == 25);
      REQUIRE(load1.front().GetStart() == Scalar(0));
      REQUIRE(load1.back().GetEnd() == Scalar(25));
      auto load2 = load(&cache, 50, 100, SnapshotLimit::FromHead(25));
      REQUIRE(load2.size() == 25);
      REQUIRE(load2.front().GetStart() == Scalar(49));
      REQUIRE(load2.back().GetEnd() == Scalar(74));
      auto load3 = load(&cache, -10, 25, SnapshotLimit::FromHead(10));
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(0));
      REQUIRE(load3.back().GetEnd() == Scalar(10));
      auto load4 = load(&cache, 95, 110, SnapshotLimit::FromHead(10));
      REQUIRE(load4.size() == 7);
      REQUIRE(load4.front().GetStart() == Scalar(94));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
    });
  }

  TEST_CASE("cache_model_loads_from_tail") {
    run_test([] {
      auto model = create_model();
      auto cache = CachedChartModel(*model);
      model->load(Scalar(0), Scalar(100), SnapshotLimit::Unlimited());
      auto load1 = load(&cache, 0, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load1.size() == 25);
      REQUIRE(load1.front().GetStart() == Scalar(76));
      REQUIRE(load1.back().GetEnd() == Scalar(101));
      auto load2 = load(&cache, 50, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load2.size() == 25);
      REQUIRE(load2.front().GetStart() == Scalar(76));
      REQUIRE(load2.back().GetEnd() == Scalar(101));
      auto load3 = load(&cache, -10, 25, SnapshotLimit::FromTail(10));
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(16));
      REQUIRE(load3.back().GetEnd() == Scalar(26));
      auto load4 = load(&cache, 95, 110, SnapshotLimit::FromTail(10));
      REQUIRE(load4.size() == 7);
      REQUIRE(load4.front().GetStart() == Scalar(94));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
    });
  }

  TEST_CASE("empty_intervals_with_head_limit") {
    run_test([] {
      auto model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
        Scalar::Type::MONEY,
        std::vector<Candlestick>({make(30, 40), make(39, 40)}));
      auto cache1 = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 40, 40, SnapshotLimit::FromHead(1));
      auto cache_sticks = load(&cache1, 40, 40, SnapshotLimit::FromHead(1));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(model.get(), 40, 40, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache1, 40, 40, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
    });
  }

  TEST_CASE("empty_intervals_with_tail_limit") {
    run_test([] {
      auto model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
        Scalar::Type::MONEY,
        std::vector<Candlestick>({make(30, 40), make(39, 40)}));
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 40, 40, SnapshotLimit::FromTail(1));
      auto cache_sticks = load(&cache, 40, 40, SnapshotLimit::FromTail(1));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(model.get(), 40, 40, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache, 40, 40, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
    });
  }

  TEST_CASE("coincident_open_intervals") {
    run_test([] {
      auto model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
        Scalar::Type::MONEY,
        std::vector<Candlestick>({make(20, 39), make(39, 40), make(39, 40),
          make(39, 40), make(39, 60)}));
      auto cache = CachedChartModel(*model);
      auto model_sticks = load(model.get(), 20, 40, SnapshotLimit::FromHead(1));
      auto cache_sticks = load(&cache, 20, 40, SnapshotLimit::FromHead(1));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(model.get(), 40, 60, SnapshotLimit::FromTail(1));
      cache_sticks = load(&cache, 40, 60, SnapshotLimit::FromTail(1));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(model.get(), 39, 39, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache, 39, 39, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
    });
  }

  TEST_CASE("limits_with_no_data") {
    run_test([] {
      auto model = std::make_shared<LocalChartModel>(Scalar::Type::MONEY,
        Scalar::Type::MONEY, std::vector<Candlestick>());
      auto test_model = TestChartModel(model->get_x_axis_type(),
        model->get_y_axis_type());
      auto cache = CachedChartModel(test_model);
      auto load1 = cache.load(Scalar(40 * Money::ONE), Scalar(50 * Money::ONE),
        SnapshotLimit::FromHead(1));
      auto cache_load1 = wait(test_model.pop_load());
      REQUIRE(cache_load1->get_first() == Scalar(40 * Money::ONE));
      REQUIRE(cache_load1->get_last() == Scalar(50 * Money::ONE));
      cache_load1->set_result({});
      wait(std::move(load1));
      auto load2 = cache.load(Scalar(30 * Money::ONE), Scalar(45 * Money::ONE),
        SnapshotLimit::FromHead(1));
      auto cache_load2 = wait(test_model.pop_load());
      REQUIRE(cache_load2->get_first() == Scalar(30 * Money::ONE));
      REQUIRE(cache_load2->get_last() == Scalar(40 * Money::ONE));
      cache_load2->set_result({});
    });
  }
}
