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

SelectConnection Spire::Styles::select(const ComponentSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return {};
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
