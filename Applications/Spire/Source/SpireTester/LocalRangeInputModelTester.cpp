#include <catch2/catch.hpp>
#include "Spire/Spire/LocalRangeInputModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_CASE("test_min_max_values_integers", "[LocalRangeInputModel]") {
  auto model = LocalRangeInputModel({Scalar(1), Scalar(2), Scalar(3), Scalar(4),
    Scalar(5)});
  REQUIRE(model.get_minimum_value() == Scalar(1));
  REQUIRE(model.get_maximum_value() == Scalar(5));
  print_test_name("test_min_max_values_integers");
}

TEST_CASE("test_min_max_values_floating_point", "[LocalRangeInputModel]") {
  auto model = LocalRangeInputModel({Scalar(1.0), Scalar(1.5), Scalar(2.0),
    Scalar(2.5), Scalar(3.0)});
  REQUIRE(model.get_minimum_value() == Scalar(1.0));
  REQUIRE(model.get_maximum_value() == Scalar(3.0));
  print_test_name("test_min_max_values_floating_point");
}

TEST_CASE("test_histogram_integers", "[LocalRangeInputModel]") {
  auto data = [] {
      auto data = std::vector<Scalar>();
      for(auto i = 1; i < 101; ++i) {
        data.push_back(Scalar(i));
      }
      return data;
    }();
  auto model = LocalRangeInputModel(data);
  auto histogram1 = model.make_histogram(5);
  REQUIRE(histogram1.m_highest_frequency == 20);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 20);
  }
  auto histogram2 = model.make_histogram(7);
  REQUIRE(histogram2.m_highest_frequency == 15);
  REQUIRE(histogram2.m_histogram[0] == 15);
  for(auto i = std::size_t(1); i < histogram2.m_histogram.size() - 1; ++i) {
    REQUIRE(histogram2.m_histogram[i] == 14);
  }
  REQUIRE(histogram2.m_histogram[6] == 15);
  auto histogram3 = model.make_histogram(10);
  REQUIRE(histogram3.m_highest_frequency == 10);
  for(auto bin : histogram3.m_histogram) {
    REQUIRE(bin == 10);
  }
  print_test_name("test_histogram_integers");
}

TEST_CASE("test_histogram_floating_point", "[LocalRangeInputModel]") {
  auto data = [] {
      auto data = std::vector<Scalar>();
      for(auto i = 1.0; i < 101.0; ++i) {
        data.push_back(Scalar(i));
      }
      return data;
    }();
  auto model = LocalRangeInputModel(data);
  auto histogram1 = model.make_histogram(5);
  REQUIRE(histogram1.m_highest_frequency == 20);
  for(auto bin : histogram1.m_histogram) {
    REQUIRE(bin == 20);
  }
  auto histogram2 = model.make_histogram(7);
  REQUIRE(histogram2.m_highest_frequency == 15);
  REQUIRE(histogram2.m_histogram[0] == 15);
  for(auto i = std::size_t(1); i < histogram2.m_histogram.size() - 1; ++i) {
    REQUIRE(histogram2.m_histogram[i] == 14);
  }
  REQUIRE(histogram2.m_histogram[6] == 15);
  auto histogram3 = model.make_histogram(10);
  REQUIRE(histogram3.m_highest_frequency == 10);
  for(auto bin : histogram3.m_histogram) {
    REQUIRE(bin == 10);
  }
  print_test_name("test_histogram_floating_point");
}
