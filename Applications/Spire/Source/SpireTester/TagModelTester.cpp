#include <doctest/doctest.h>
#include <vector>
#include "Spire/KeyBindings/ConjunctionTerm.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/SatisfiedTerm.hpp"
#include "Spire/KeyBindings/TagModel.hpp"
#include "Spire/KeyBindings/TagPresentTerm.hpp"

using namespace Spire;

namespace {
  auto make_tag(int key) {
    return KeyBindings::Tag(key, 0);
  }

  void require_tags(const std::vector<KeyBindings::Tag>& tags,
      const std::vector<int>& keys) {
    for(auto key : keys) {
      REQUIRE(std::find_if(tags.begin(), tags.end(), [&] (auto& tag) {
        return tag.get_key() == key;
      }) != tags.end());
    }
  }
}

TEST_SUITE("TagModel") {
  TEST_CASE("get_name") {
    auto tag_model = TagModel({{"a", make_tag(1),
      std::make_shared<SatisfiedTerm>()}, {"b", make_tag(2),
      std::make_shared<SatisfiedTerm>()}});
    REQUIRE(tag_model.get_tag_name(1) == "a");
    REQUIRE(tag_model.get_tag_name(2) == "b");
  }

  TEST_CASE("load_enabled_tags") {
    auto tag_model = TagModel({
      {"a", make_tag(1), std::make_shared<SatisfiedTerm>()},
      {"b", make_tag(2), std::make_shared<TagPresentTerm>(1)},
      {"c", make_tag(3), std::make_shared<ConjunctionTerm>(
        std::vector<std::shared_ptr<Term>>{
        std::make_shared<TagPresentTerm>(1),
        std::make_shared<TagPresentTerm>(4)})}});
    require_tags(tag_model.load_satisfied_tags({}), {1});
    require_tags(tag_model.load_satisfied_tags({make_tag(1)}), {1, 2});
    require_tags(tag_model.load_satisfied_tags({make_tag(1), make_tag(4)}),
      {1, 2, 3});
  }
}
