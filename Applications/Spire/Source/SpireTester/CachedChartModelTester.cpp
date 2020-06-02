#include <tuple>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/SpireTester/TestChartModel.hpp"

using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  class TestLocalChartModel : public LocalChartModel {
    public:
      using LocalChartModel::LocalChartModel;

      QtPromise<std::vector<Candlestick>> load(Scalar first, Scalar last,
          const Beam::Queries::SnapshotLimit& limit) override {
        m_ranges.push_back(std::tuple(first, last));
        return LocalChartModel::load(first, last, limit);
      }

      void require_load_ranges(
          const std::vector<std::tuple<Scalar, Scalar>>& ranges) {
        REQUIRE(m_ranges == ranges);
        m_ranges.clear();
      }

    private:
      std::vector<std::tuple<Scalar, Scalar>> m_ranges;
  };

  auto create_candlesticks() {
    auto candlesticks = std::vector<Candlestick>();
    for(auto i = 0; i < 101; ++i) {
      candlesticks.push_back({Scalar(i * Money::ONE),
        Scalar((i + 1) * Money::ONE)});
    }
    return candlesticks;
  }

  auto create_custom_reference_model(std::vector<Candlestick> candlesticks) {
    return LocalChartModel(Scalar::Type::MONEY, Scalar::Type::MONEY,
      std::move(candlesticks));
  }

  auto create_reference_model() {
    return create_custom_reference_model(create_candlesticks());
  }

  auto create_custom_local_model(std::vector<Candlestick> candlesticks) {
    return TestLocalChartModel(Scalar::Type::MONEY, Scalar::Type::MONEY,
      std::move(candlesticks));
  }

  auto create_local_model() {
    return create_custom_local_model(create_candlesticks());
  }

  auto load(ChartModel* model, int first, int last,
      const SnapshotLimit& limit) {
    return wait(model->load(Scalar(first * Money::ONE),
      Scalar(last * Money::ONE), limit));
  }

  auto make_query(int first, int last) {
    return std::tuple(Scalar(first * Money::ONE), Scalar(last * Money::ONE),
      SnapshotLimit::Unlimited());
  }

  auto make_range(int first, int last) {
    return std::tuple(Scalar(first * Money::ONE), Scalar(last * Money::ONE));
  }

  auto make(int start, int end) {
    return Candlestick(Scalar(start), Scalar(end));
  }

  void require_no_waiting(TestChartModel& test_model, ChartModel& reference_model,
      const std::vector<std::tuple<Scalar, Scalar, SnapshotLimit>>& queries) {
    auto indexes = std::vector<std::size_t>(queries.size());
    for(auto i = std::size_t(0); i < indexes.size(); ++i) {
      indexes[i] = i;
    }
    do {
      auto cached_model = CachedChartModel(test_model);
      auto promises = std::vector<QtPromise<std::vector<Candlestick>>>();
      auto operations = std::vector<std::shared_ptr<
        TestChartModel::LoadEntry>>();
      for(auto [first, last, limit] : queries) {
        promises.push_back(cached_model.load(first, last, limit));
        operations.push_back(wait(test_model.pop_load()));
      }
      for(auto i : indexes) {
        auto& operation = operations[i];
        operation->set_result(wait(reference_model.load(operation->get_first(),
          operation->get_last(), operation->get_limit())));
        wait(std::move(promises[i]));
      }
    } while(std::next_permutation(indexes.begin(), indexes.end()));
  }
}

TEST_SUITE("CachedChartModel") {
  TEST_CASE("right_no_overlap") {
    run_test([] {
      auto model = create_reference_model();
      auto cache = CachedChartModel(model);
      auto model_sticks = load(&model, 10, 20, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 10, 20, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(&model, 30, 40, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 30, 40, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("left_no_overlap") {
    run_test([] {
      auto model = create_reference_model();
      auto cache = CachedChartModel(model);
      auto model_sticks = load(&model, 20, 30, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 20, 30, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(&model, 10, 18, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 10, 18, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
    });
  }

  TEST_CASE("right_overlap") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 25, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      test_model.require_load_ranges({make_range(25, 50)});
      model_sticks = load(&model, 40, 60, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(50, 60)});
    });
  }

  TEST_CASE("left_overlap") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 25, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      test_model.require_load_ranges({make_range(25, 50)});
      model_sticks = load(&model, 10, 30, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 10, 30, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(10, 25)});
    });
  }

  TEST_CASE("subset") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 25, 75, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 25, 75, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      test_model.require_load_ranges({make_range(25, 75)});
      model_sticks = load(&model, 40, 60, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({});
    });
  }

  TEST_CASE("single_superset") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 40, 60, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      test_model.require_load_ranges({make_range(40, 60)});
      REQUIRE(model_sticks == cached_sticks);
      model_sticks = load(&model, 20, 80, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(20, 40),
        make_range(60, 80)});
    });
  }

  TEST_CASE("multiple_supersets") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 40, 60, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      test_model.require_load_ranges({make_range(40, 60)});
      model_sticks = load(&model, 20, 80, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 20, 80, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(20, 40),
        make_range(60, 80)});
      model_sticks = load(&model, 10, 90, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 10, 90, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(10, 20),
        make_range(80, 90)});
    });
  }

  TEST_CASE("multiple_subsets_and_supersets") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 40, 50, SnapshotLimit::Unlimited());
      auto cached_sticks = load(&cache, 40, 50, SnapshotLimit::Unlimited());
      REQUIRE(model_sticks == cached_sticks);
      test_model.require_load_ranges({make_range(40, 50)});
      model_sticks = load(&model, 60, 70, SnapshotLimit::Unlimited());
      auto test_sticks = load(&cache, 60, 70, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(60, 70)});
      model_sticks = load(&model, 80, 90, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 80, 90, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(80, 90)});
      model_sticks = load(&model, 45, 72, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 45, 72, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(50, 60),
        make_range(70, 72)});
      model_sticks = load(&model, 35, 95, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 35, 95, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({make_range(35, 40),
        make_range(72, 80), make_range(90, 95)});
      model_sticks = load(&model, 40, 60, SnapshotLimit::Unlimited());
      test_sticks = load(&cache, 40, 60, SnapshotLimit::Unlimited());
      REQUIRE(test_sticks == model_sticks);
      test_model.require_load_ranges({});
    });
  }

  TEST_CASE("multiple_cache_hits") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = TestChartModel(model.get_x_axis_type(),
        model.get_y_axis_type());
      auto cache = CachedChartModel(test_model);
      auto pre_load1 = cache.load(Scalar(40 * Money::ONE),
        Scalar(50 * Money::ONE), SnapshotLimit::Unlimited());
      wait(test_model.pop_load())->set_result(wait(
        model.load(Scalar(40 * Money::ONE), Scalar(50 * Money::ONE),
        SnapshotLimit::Unlimited())));
      wait(std::move(pre_load1));
      auto pre_load2 = cache.load(Scalar(60 * Money::ONE),
        Scalar(70 * Money::ONE), SnapshotLimit::Unlimited());
      wait(test_model.pop_load())->set_result(wait(
        model.load(Scalar(60 * Money::ONE), Scalar(70 * Money::ONE),
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
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
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
      auto model = create_reference_model();
      auto test_model = create_local_model();
      auto cache = CachedChartModel(test_model);
      auto load1 = load(&cache, 0, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load1.size() == 25);
      REQUIRE(load1.front().GetStart() == Scalar(76));
      REQUIRE(load1.back().GetEnd() == Scalar(101));
      test_model.require_load_ranges({make_range(0, 100)});
      auto load2 = load(&cache, 50, 100, SnapshotLimit::FromTail(25));
      REQUIRE(load2.size() == 25);
      REQUIRE(load2.front().GetStart() == Scalar(76));
      REQUIRE(load2.back().GetEnd() == Scalar(101));
      test_model.require_load_ranges({});
      auto load3 = load(&cache, -10, 25, SnapshotLimit::FromTail(10));
      REQUIRE(load3.size() == 10);
      REQUIRE(load3.front().GetStart() == Scalar(16));
      REQUIRE(load3.back().GetEnd() == Scalar(26));
      test_model.require_load_ranges({make_range(-10, 25)});
      auto load4 = load(&cache, 95, 110, SnapshotLimit::FromTail(10));
      REQUIRE(load4.size() == 7);
      REQUIRE(load4.front().GetStart() == Scalar(94));
      REQUIRE(load4.back().GetEnd() == Scalar(101));
      test_model.require_load_ranges({make_range(100, 110)});
    });
  }

  TEST_CASE("empty_intervals_with_head_limit") {
    run_test([] {
      auto candlesticks = std::vector<Candlestick>{make(30, 40), make(39, 40)};
      auto model = create_custom_reference_model(candlesticks);
      auto test_model = create_custom_local_model(std::move(candlesticks));
      auto cache1 = CachedChartModel(test_model);
      auto model_sticks = load(&model, 40, 40, SnapshotLimit::FromHead(1));
      auto cache_sticks = load(&cache1, 40, 40, SnapshotLimit::FromHead(1));
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(40, 40)});
      model_sticks = load(&model, 40, 40, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache1, 40, 40, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(40, 40)});
    });
  }

  TEST_CASE("empty_intervals_with_tail_limit") {
    run_test([] {
      auto candlesticks = std::vector<Candlestick>{make(30, 40), make(39, 40)};
      auto model = create_custom_reference_model(candlesticks);
      auto test_model = create_custom_local_model(std::move(candlesticks));
      auto cache1 = CachedChartModel(test_model);
      auto model_sticks = load(&model, 40, 40, SnapshotLimit::FromTail(1));
      auto cache_sticks = load(&cache1, 40, 40, SnapshotLimit::FromTail(1));
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(40, 40)});
      model_sticks = load(&model, 40, 40, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache1, 40, 40, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(40, 40)});
    });
  }

  TEST_CASE("coincident_open_intervals") {
    run_test([] {
      auto candlesticks = std::vector<Candlestick>{make(20, 39), make(39, 40),
        make(39, 40), make(39, 40), make(39, 60)};
      auto model = create_custom_reference_model(candlesticks);
      auto test_model = create_custom_local_model(std::move(candlesticks));
      auto cache = CachedChartModel(test_model);
      auto model_sticks = load(&model, 20, 40, SnapshotLimit::FromHead(1));
      auto cache_sticks = load(&cache, 20, 40, SnapshotLimit::FromHead(1));
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(20, 40)});
      model_sticks = load(&model, 40, 60, SnapshotLimit::FromTail(1));
      cache_sticks = load(&cache, 40, 60, SnapshotLimit::FromTail(1));
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(40, 60)});
      model_sticks = load(&model, 39, 39, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache, 39, 39, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
      test_model.require_load_ranges({make_range(39, 39)});
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

  TEST_CASE("candlesticks_between_loaded_open_intervals") {
    run_test([] {
      auto candlesticks = std::vector<Candlestick>{{make(20, 30), make(40, 41),
        make(40, 42), make(40, 43), make(50, 60)}};
      auto model = create_custom_reference_model(candlesticks);
      auto test_model = create_custom_local_model(std::move(candlesticks));
      auto cache = CachedChartModel(model);
      auto model_sticks = load(&model, 20, 40, SnapshotLimit::FromHead(2));
      auto cache_sticks = load(&cache, 20, 40, SnapshotLimit::FromHead(2));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(&model, 40, 60, SnapshotLimit::FromTail(2));
      cache_sticks = load(&cache, 40, 60, SnapshotLimit::FromTail(2));
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(&model, 40, 40, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache, 40, 40, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
      model_sticks = load(&model, 20, 60, SnapshotLimit::Unlimited());
      cache_sticks = load(&cache, 20, 60, SnapshotLimit::Unlimited());
      REQUIRE(cache_sticks == model_sticks);
    });
  }

  TEST_CASE("no_waiting_concurrent_loads") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = TestChartModel(model.get_x_axis_type(),
        model.get_y_axis_type());
      require_no_waiting(test_model, model, {make_query(5, 20),
        make_query(30, 40), make_query(45, 50), make_query(90, 200)});
    });
  }

  TEST_CASE("no_waiting_adjacent_loads") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = TestChartModel(model.get_x_axis_type(),
        model.get_y_axis_type());
      require_no_waiting(test_model, model, {make_query(5, 20),
        make_query(20, 40), make_query(40, 50), make_query(50, 200)});
    });
  }

  TEST_CASE("load_head_before_waiting") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = TestChartModel(model.get_x_axis_type(),
        model.get_y_axis_type());
      auto cache = CachedChartModel(test_model);
      auto subset = cache.load(Scalar(40 * Money::ONE),
        Scalar(50 * Money::ONE), SnapshotLimit::Unlimited());
      auto load1 = wait(test_model.pop_load());
      auto result = cache.load(Scalar(30 * Money::ONE),
        Scalar(60 * Money::ONE), SnapshotLimit::Unlimited());
      auto load2 = wait(test_model.pop_load());
      REQUIRE(load2->get_first() == Scalar(30 * Money::ONE));
      REQUIRE(load2->get_last() == Scalar(40 * Money::ONE));
      load2->set_result(load(&model, 30, 40, SnapshotLimit::Unlimited()));
      load1->set_result(load(&model, 40, 50, SnapshotLimit::Unlimited()));
      auto load3 = wait(test_model.pop_load());
      REQUIRE(load3->get_first() == Scalar(50 * Money::ONE));
      REQUIRE(load3->get_last() == Scalar(60 * Money::ONE));
      load3->set_result(load(&model, 50, 60, SnapshotLimit::Unlimited()));
      auto candlesticks = wait(std::move(result));
      REQUIRE(candlesticks == load(&model, 30, 60,
        SnapshotLimit::Unlimited()));
    });
  }

  TEST_CASE("load_tail_before_waiting") {
    run_test([] {
      auto model = create_reference_model();
      auto test_model = TestChartModel(model.get_x_axis_type(),
        model.get_y_axis_type());
      auto cache = CachedChartModel(test_model);
      auto subset = cache.load(Scalar(40 * Money::ONE),
        Scalar(50 * Money::ONE), SnapshotLimit::FromTail(100));
      auto load1 = wait(test_model.pop_load());
      auto result = cache.load(Scalar(30 * Money::ONE),
        Scalar(60 * Money::ONE), SnapshotLimit::FromTail(100));
      auto load2 = wait(test_model.pop_load());
      REQUIRE(load2->get_first() == Scalar(50 * Money::ONE));
      REQUIRE(load2->get_last() == Scalar(60 * Money::ONE));
      load2->set_result(load(&model, 50, 60, SnapshotLimit::FromTail(100)));
      load1->set_result(load(&model, 40, 50, SnapshotLimit::FromTail(100)));
      auto load3 = wait(test_model.pop_load());
      REQUIRE(load3->get_first() == Scalar(30 * Money::ONE));
      REQUIRE(load3->get_last() == Scalar(40 * Money::ONE));
      load3->set_result(load(&model, 30, 40, SnapshotLimit::FromTail(100)));
      auto candlesticks = wait(std::move(result));
      REQUIRE(candlesticks == load(&model, 30, 60,
        SnapshotLimit::FromTail(100)));
    });
  }

  TEST_CASE("load_unsaturated_limit") {
    run_test([] {
      auto test_model = TestChartModel(Scalar::Type::TIMESTAMP,
        Scalar::Type::MONEY);
      auto cache = CachedChartModel(test_model);
      auto t1 = ptime(date(2020, 5, 1), time_duration(1, 0, 0));
      auto cache_load1 = cache.load(Scalar(t1), Scalar(t1 + seconds(5)),
        SnapshotLimit::FromHead(3));
      auto test_load1 = wait(test_model.pop_load());
      REQUIRE(test_load1->get_first() == Scalar(t1));
      REQUIRE(test_load1->get_last() == Scalar(t1 + seconds(5)));
      REQUIRE(test_load1->get_limit() == SnapshotLimit::FromHead(3));
      auto test_result1 = std::vector<Candlestick>();
      test_result1.emplace_back(Scalar(t1), Scalar(t1 + seconds(1)));
      test_result1.emplace_back(Scalar(t1 + seconds(1)),
        Scalar(t1 + seconds(2)));
      test_result1.emplace_back(Scalar(t1 + seconds(3)),
        Scalar(t1 + seconds(4)));
      test_load1->set_result(test_result1);
      auto cache_result1 = wait(std::move(cache_load1));
      REQUIRE(cache_result1.size() == 3);
      auto cache_load2 = cache.load(Scalar(t1), Scalar(t1 + seconds(5)),
        SnapshotLimit::FromHead(4));
      auto test_load2 = wait(test_model.pop_load());
      REQUIRE(test_load2->get_first() == Scalar(t1 + seconds(3)));
      REQUIRE(test_load2->get_last() == Scalar(t1 + seconds(5)));
      REQUIRE(test_load2->get_limit() == SnapshotLimit::FromHead(2));
      test_load2->set_result({});
    });
  }

  TEST_CASE("load_saturated_limit") {
    run_test([] {
      auto test_model = TestChartModel(Scalar::Type::TIMESTAMP,
        Scalar::Type::MONEY);
      auto cache = CachedChartModel(test_model);
      auto t1 = ptime(date(2020, 5, 1), time_duration(1, 0, 0));
      auto cache_load1 = cache.load(Scalar(t1), Scalar(t1 + seconds(5)),
        SnapshotLimit::FromHead(3));
      auto test_load1 = wait(test_model.pop_load());
      REQUIRE(test_load1->get_first() == Scalar(t1));
      REQUIRE(test_load1->get_last() == Scalar(t1 + seconds(5)));
      REQUIRE(test_load1->get_limit() == SnapshotLimit::FromHead(3));
      auto test_result1 = std::vector<Candlestick>();
      test_result1.emplace_back(Scalar(t1), Scalar(t1 + seconds(1)));
      test_result1.emplace_back(Scalar(t1 + seconds(2)),
        Scalar(t1 + seconds(3)));
      test_result1.emplace_back(Scalar(t1 + seconds(2)),
        Scalar(t1 + seconds(4)));
      test_load1->set_result(test_result1);
      auto cache_result1 = wait(std::move(cache_load1));
      REQUIRE(cache_result1.size() == 3);
      auto cache_load2 = cache.load(Scalar(t1), Scalar(t1 + seconds(5)),
        SnapshotLimit::FromHead(4));
      auto test_load2 = wait(test_model.pop_load());
      REQUIRE(test_load2->get_first() == Scalar(t1 + seconds(2)));
      REQUIRE(test_load2->get_last() == Scalar(t1 + seconds(5)));
      REQUIRE(test_load2->get_limit() == SnapshotLimit::FromHead(3));
      test_load2->set_result({});
    });
  }
}
