#include <doctest/doctest.h>
#include "Spire/KeyBindings/ConjunctionTerm.hpp"
#include "Spire/KeyBindings/DisjunctionTerm.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/NegationTerm.hpp"
#include "Spire/KeyBindings/RegionTerm.hpp"
#include "Spire/KeyBindings/SatisfiedTerm.hpp"
#include "Spire/KeyBindings/TagEqualTerm.hpp"
#include "Spire/KeyBindings/TagPresentTerm.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("Term") {
  TEST_CASE("satisfied_term") {
    auto term = SatisfiedTerm();
    REQUIRE(term.is_satisfied({}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 1),
      KeyBindings::Tag(10, 5)}));
  }

  TEST_CASE("tag_present_term") {
    auto term = TagPresentTerm(10);
    REQUIRE(!term.is_satisfied({}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 1),
      KeyBindings::Tag(15, 5)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 1),
      KeyBindings::Tag(10, 5)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(10, 5)}));
  }

  TEST_CASE("tag_equal_term") {
    auto term = TagEqualTerm(KeyBindings::Tag(5, 10));
    REQUIRE(!term.is_satisfied({}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, static_cast<char>(10)),
      KeyBindings::Tag(15, 5)}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 10.),
      KeyBindings::Tag(10, 10)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 10)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(1, 8.),
      KeyBindings::Tag(5, 10), KeyBindings::Tag(15, 'c')}));
  }

  TEST_CASE("region_term") {
    auto term = RegionTerm(Region(CountryCode(4)));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 10)}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, Region::Global())}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, boost::optional<Region>())}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, Region(CountryCode(4)))}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, Region(Security("MSFT", CountryCode(4))))}));
    auto global_region_term = RegionTerm(Region::Global());
    REQUIRE(global_region_term.is_satisfied({KeyBindings::Tag(5, 10)}));
    REQUIRE(global_region_term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, boost::optional<Region>())}));
    REQUIRE(global_region_term.is_satisfied({KeyBindings::Tag(5, 10),
      KeyBindings::Tag(55, Region(CountryCode(4)))}));
  }

  TEST_CASE("conjunction_term") {
    auto term = ConjunctionTerm({std::make_shared<TagPresentTerm>(10),
      std::make_shared<TagEqualTerm>(KeyBindings::Tag(5, 20))});
    REQUIRE(!term.is_satisfied({}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(10, 0),
      KeyBindings::Tag(5, 5)}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(1, 1),
      KeyBindings::Tag(5, 20)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(10, 0),
      KeyBindings::Tag(5, 20), KeyBindings::Tag(1, 1)}));
  }

  TEST_CASE("conjunction_term") {
    auto term = DisjunctionTerm({std::make_shared<TagPresentTerm>(10),
      std::make_shared<TagEqualTerm>(KeyBindings::Tag(5, 20))});
    REQUIRE(!term.is_satisfied({}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(10, 0),
      KeyBindings::Tag(5, 5)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(1, 1),
      KeyBindings::Tag(5, 20)}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(10, 0),
      KeyBindings::Tag(5, 20), KeyBindings::Tag(1, 1)}));
  }

  TEST_CASE("negation_term") {
    auto term = NegationTerm(std::make_shared<TagPresentTerm>(10));
    REQUIRE(term.is_satisfied({}));
    REQUIRE(term.is_satisfied({KeyBindings::Tag(5, 1),
      KeyBindings::Tag(15, 5)}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(5, 1),
      KeyBindings::Tag(10, 5)}));
    REQUIRE(!term.is_satisfied({KeyBindings::Tag(10, 5)}));
  }
}
