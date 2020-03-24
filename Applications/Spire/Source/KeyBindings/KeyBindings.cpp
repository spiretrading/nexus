#include "Spire/KeyBindings/KeyBindings.hpp"
#include <algorithm>
#include <type_traits>

using namespace Nexus;
using namespace Spire;

void KeyBindings::set_binding(QKeySequence sequence, const Action& action) {
  auto region = std::visit([] (auto& action) {
    using Type = std::decay_t<decltype(action)>;
    if constexpr(std::is_same_v<Type, CancelAction>) {
      return Region(Region::GlobalTag());
    } else {
      return action.m_region;
    }
  }, action);
  auto i = m_bindings.find(sequence);
  if(i == m_bindings.end()) {
    i = m_bindings.insert(sequence, Actions(boost::none));
  }
  i->Set(region, action);
}

void KeyBindings::reset_binding(const Region& region,
    const QKeySequence& sequence) {
  auto i = m_bindings.find(sequence);
  if(i != m_bindings.end()) {
    auto& actions = *i;
    if(region.IsGlobal()) {
      actions.Set(region, boost::none);
    } else {
      actions.Erase(region);
      if(actions.GetSize() == 1 && actions.Get(Region(Region::GlobalTag()))) {
        m_bindings.erase(i);
      }
    }
  }
}

boost::optional<const KeyBindings::Action&>
    KeyBindings::find_binding(const Region& region,
    const QKeySequence& sequence) const {
  auto i = m_bindings.find(sequence);
  if(i == m_bindings.end()) {
    return boost::none;
  }
  auto& action = i->Get(region);
  if(action) {
    return *action;
  } else {
    return boost::none;
  }
}

KeyBindings::OrderBindingsList
    KeyBindings::build_order_bindings_list() const {
  auto list = OrderBindingsList();
  return list;
}

KeyBindings::CancelBindingsList
    KeyBindings::build_cancel_bindings_list() const {
  auto list = CancelBindingsList();
  return list;
}

KeyBindings::ActionBindingsList
    KeyBindings::build_action_bindings_list() const {
  auto list = ActionBindingsList();
  std::for_each(m_bindings.constKeyValueBegin(), m_bindings.constKeyValueEnd(),
    [&] (auto& e) {
      auto& elements = e.second;
      auto region_actions = std::vector<std::tuple<const Region,
        Actions::Element>>();
      std::copy_if(elements.Begin(), elements.End(),
        std::back_inserter(region_actions), [] (auto& element) {
          return std::get<1>(element).is_initialized();
        });
      std::transform(region_actions.begin(), region_actions.end(),
        std::back_inserter(list), [&] (auto& element) {
          return std::make_pair(e.first, *std::get<1>(element));
        });
    });
  return list;
}
