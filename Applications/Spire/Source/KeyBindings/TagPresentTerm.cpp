#include "Spire/KeyBindings/TagPresentTerm.hpp"
#include <algorithm>

using namespace Spire;

TagPresentTerm::TagPresentTerm(int key)
  : m_key(key) {}

bool TagPresentTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  return std::find_if(tags.begin(), tags.end(), [&] (auto& tag) {
    return tag.get_key() == m_tag_key;
  }) != tags.end();
}
