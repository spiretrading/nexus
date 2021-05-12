#include "Spire/Styles/ComponentSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using SelectCallable =
    std::function<std::vector<Stylist*> (const ComponentSelector&, Stylist&)>;

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

std::vector<Stylist*>
    Spire::Styles::select(const ComponentSelector& selector, Stylist& source) {
  auto i = get_registry().find(selector.get_type());
  if(i == get_registry().end()) {
    return {};
  }
  return i->second(selector, source);
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
