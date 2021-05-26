#include "Spire/Styles/ComponentSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using SelectCallable = std::function<
    std::unordered_set<Stylist*> (const ComponentSelector&, Stylist&)>;

  auto& get_registry() {
    static auto registry =
      std::unordered_map<std::type_index, SelectCallable>();
    return registry;
  }
}

std::type_index ComponentSelector::get_type() const {
  return m_id.type();
}

bool ComponentSelector::operator ==(const ComponentSelector& selector) const {
  return m_is_equal(*this, selector);
}

bool ComponentSelector::operator !=(const ComponentSelector& selector) const {
  return !(*this == selector);
}

std::size_t Spire::Styles::hash_value(const ComponentSelector& element) {
  return std::hash<std::type_index>()(element.get_type());
}

std::unordered_set<Stylist*> Spire::Styles::select(
    const ComponentSelector& selector, std::unordered_set<Stylist*> sources) {
  auto selection = std::unordered_set<Stylist*>();
  for(auto source : sources) {
    auto i = get_registry().find(selector.get_type());
    if(i != get_registry().end()) {
      auto source_selection = i->second(selector, *source);
      selection.insert(source_selection.begin(), source_selection.end());
    }
  }
  return selection;
}

bool Spire::Styles::Details::register_id(
    std::type_index type, SelectCallable callable) {
  get_registry().insert(std::pair(type, std::move(callable)));
  return true;
}

std::size_t std::hash<ComponentSelector>::operator ()(
    const ComponentSelector& selector) const {
  return hash_value(selector);
}
