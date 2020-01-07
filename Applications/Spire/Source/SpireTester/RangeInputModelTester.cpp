#include <catch.hpp>
#include "Spire/Spire/RangeInputModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_CASE("test_min_max_values_integers", "[RangeInputModel]") {
  auto model = RangeInputModel<int>({1, 2, 3, 4, 5});
  REQUIRE(model.get_minimum_value() == 1);
  REQUIRE(model.get_maximum_value() == 5);
  print_test_name("test_min_max_values_integers");
}

TEST_CASE("test_min_max_values_doubles", "[RangeInputModel]") {
  auto model = RangeInputModel<double>({1.0, 1.5, 2.0, 2.5, 3.0});
  REQUIRE(model.get_minimum_value() == 1.0);
  REQUIRE(model.get_maximum_value() == 3.0);
  print_test_name("test_min_max_values_doubles");
}

TEST_CASE("test_histogram_integers", "[RangeInputModel]") {
  auto data = [] {
      auto data = std::vector<QVariant>();
      for(auto i = 1; i < 101; ++i) {
        data.push_back(i);
      }
      return data;
    }();
  auto model = RangeInputModel<int>(data);
  auto histogram1 = model.make_histogram(5);
  REQUIRE(histogram1.m_highest_frequency == 20);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 20);
  }
  auto histogram2 = model.make_histogram(7);
  REQUIRE(histogram2.m_highest_frequency == 15);
  for(auto i = std::size_t(0); i < histogram2.m_histogram.size() - 1; ++i) {
    REQUIRE(histogram2.m_histogram[i] == 14);
  }
  REQUIRE(histogram2.m_histogram[6] == 15);
  auto histogram3 = model.make_histogram(10);
  REQUIRE(histogram1.m_highest_frequency == 10);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 10);
  }
  print_test_name("test_histogram_integers");
}

TEST_CASE("test_histogram_doubles", "[RangeInputModel]") {
  auto data = [] {
      auto data = std::vector<QVariant>();
      for(auto i = 1.0; i < 101.0; ++i) {
        data.push_back(i);
      }
      return data;
    }();
  auto model = RangeInputModel<double>(data);
  auto histogram1 = model.make_histogram(5);
  REQUIRE(histogram1.m_highest_frequency == 20);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 20.0);
  }
  auto histogram2 = model.make_histogram(7);
  REQUIRE(histogram2.m_highest_frequency == 15);
  for(auto i = std::size_t(0); i < histogram2.m_histogram.size() - 1; ++i) {
    REQUIRE(histogram2.m_histogram[i] == 14);
  }
  REQUIRE(histogram2.m_histogram[6] == 15);
  auto histogram3 = model.make_histogram(10);
  REQUIRE(histogram1.m_highest_frequency == 10);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 10);
  }
  print_test_name("test_histogram_doubles");
}
