#include "Spire/KeyBindings/TagEqualTerm.hpp"
#include <algorithm>

using namespace Spire;

TagEqualTerm::TagEqualTerm(const KeyBindings::Tag& tag)
  : m_tag(tag) {}

bool TagEqualTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  return std::find(tags.begin(), tags.end(), m_tag) != tags.end();
}
