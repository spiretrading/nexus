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
  std::for_each(m_bindings.constKeyValueBegin(), m_bindings.constKeyValueEnd(),
    [&] (auto& e) {
      auto& elements = e.second;
      for(auto i = elements.Begin(); i != elements.End(); ++i) {
        auto& action = std::get<1>(*i);
        if(action) {
          auto order_action = std::get_if<OrderAction>(&(*action));
          if(order_action != nullptr) {
            list.emplace_back(e.first, *order_action);
          }
        }
      }
    });
  return list;
}

KeyBindings::CancelBindingsList
    KeyBindings::build_cancel_bindings_list() const {
  auto list = CancelBindingsList();
  std::for_each(m_bindings.constKeyValueBegin(), m_bindings.constKeyValueEnd(),
    [&] (auto& e) {
      auto& elements = e.second;
      for(auto i = elements.Begin(); i != elements.End(); ++i) {
        auto& action = std::get<1>(*i);
        if(action) {
          auto cancel_action = std::get_if<CancelAction>(&(*action));
          if(cancel_action != nullptr) {
            list.emplace_back(e.first, *cancel_action);
          }
        }
      }
    });
  return list;
}

KeyBindings::ActionBindingsList
    KeyBindings::build_action_bindings_list() const {
  auto list = ActionBindingsList();
  std::for_each(m_bindings.constKeyValueBegin(), m_bindings.constKeyValueEnd(),
    [&] (auto& e) {
      auto& elements = e.second;
      for(auto i = elements.Begin(); i != elements.End(); ++i) {
        auto& action = std::get<1>(*i);
        if(action) {
          list.emplace_back(e.first, *action);
        }
      }
    });
  return list;
}
