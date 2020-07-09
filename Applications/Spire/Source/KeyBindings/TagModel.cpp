#include "Spire/KeyBindings/TagModel.hpp"
#include <algorithm>

using namespace Spire;

TagModel::TagModel(std::vector<Schema> schemas)
  : m_schemas(std::move(schemas)) {}

std::string TagModel::get_tag_name(int tag_key) const {
  return std::find_if(m_schemas.begin(), m_schemas.end(), [=] (auto& schema) {
    return schema.m_tag.get_key() == tag_key;
  })->m_name;
}

std::vector<KeyBindings::Tag> TagModel::load_enabled_tags(
    const std::vector<KeyBindings::Tag>& prior_tags) const {
  auto result = std::vector<KeyBindings::Tag>();
  for(auto& schema : m_schemas) {
    if(schema.m_requirement->is_satisfied(prior_tags)) {
      result.push_back(schema.m_tag);
    }
  }
  return result;
}
