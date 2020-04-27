#include "Spire/KeyBindings/KeyBindings.hpp"
#include <algorithm>
#include <type_traits>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/optional/optional_io.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Nexus;
using namespace Spire;

bool KeyBindings::CancelActionBinding::operator ==(
    const CancelActionBinding& rhs) const {
  return std::tie(m_sequence, m_region, m_action) ==
    std::tie(rhs.m_sequence, rhs.m_region, rhs.m_action);
}

bool KeyBindings::CancelActionBinding::operator !=(
    const CancelActionBinding& rhs) const {
  return !(*this == rhs);
}

KeyBindings KeyBindings::get_default_key_bindings() {
  auto bindings = KeyBindings();
  auto region = Region::Global();
  auto action1 = KeyBindings::OrderAction{"", OrderType::LIMIT, Side::BID,
    TimeInForce(TimeInForce::Type::NONE), 100, {}};
  bindings.set(Qt::Key_F5, region, action1);
  auto action2 = KeyBindings::OrderAction{"", OrderType::LIMIT, Side::ASK,
    TimeInForce(TimeInForce::Type::NONE), 100, {}};
  bindings.set(Qt::Key_F6, region, action2);
  auto action3 = KeyBindings::OrderAction{"", OrderType::MARKET, Side::BID,
    TimeInForce(TimeInForce::Type::NONE), 100, {}};
  bindings.set(Qt::Key_F7, region, action3);
  auto action4 = KeyBindings::OrderAction{"", OrderType::MARKET, Side::ASK,
    TimeInForce(TimeInForce::Type::NONE), 100, {}};
  bindings.set(Qt::Key_F8, region, action4);
  return bindings;
}

void KeyBindings::set(QKeySequence sequence, const Region& region,
    const Action& action) {
  auto i = std::find_if(m_bindings.begin(), m_bindings.end(),
    [&] (auto mapping) {
      return mapping.m_key_sequence == sequence;
    });
  if(i == m_bindings.end()) {
    m_bindings.push_back({sequence, Actions(boost::none)});
    m_bindings.back().m_actions.Set(region, action);
  } else {
    i->m_actions.Set(region, action);
  }
}

void KeyBindings::reset(const Region& region, const QKeySequence& sequence) {
  auto i = std::find_if(m_bindings.begin(), m_bindings.end(),
    [&] (auto mapping) {
      return mapping.m_key_sequence == sequence;
    });
  if(i != m_bindings.end()) {
    auto& actions = i->m_actions;
    if(region.IsGlobal()) {
      actions.Set(region, boost::none);
    } else {
      actions.Erase(region);
      m_bindings.erase(i, i);
    }
  }
}

boost::optional<KeyBindings::Action> KeyBindings::find(const Region& region,
    const QKeySequence& sequence) const {
  auto i = std::find_if(m_bindings.begin(), m_bindings.end(),
    [&] (auto mapping) {
      return mapping.m_key_sequence == sequence;
    });
  if(i == m_bindings.end()) {
    return boost::none;
  }
  return i->m_actions.Get(region);
}

std::vector<KeyBindings::OrderActionBinding>
    KeyBindings::build_order_bindings() const {
  auto list = std::vector<OrderActionBinding>();
  for(auto& mapping : m_bindings) {
    auto& elements = mapping.m_actions;
    for(auto i = elements.Begin(); i != elements.End(); ++i) {
      if(auto& action = std::get<1>(*i)) {
        if(auto order_action = std::get_if<OrderAction>(&(*action))) {
          list.push_back({mapping.m_key_sequence, std::get<0>(*i),
            *order_action});
        }
      }
    }
  }
  return list;
}

std::vector<KeyBindings::CancelActionBinding>
    KeyBindings::build_cancel_bindings() const {
  auto list = std::vector<CancelActionBinding>();
  for(auto& mapping : m_bindings) {
    auto& elements = mapping.m_actions;
    for(auto i = elements.Begin(); i != elements.End(); ++i) {
      if(auto& action = std::get<1>(*i)) {
        if(auto cancel_action = std::get_if<CancelAction>(&(*action))) {
          list.push_back({mapping.m_key_sequence, std::get<0>(*i),
            *cancel_action});
        }
      }
    }
  }
  return list;
}

std::vector<KeyBindings::ActionBinding>
    KeyBindings::build_action_bindings() const {
  auto list = std::vector<ActionBinding>();
  for(auto& mapping : m_bindings) {
    auto& elements = mapping.m_actions;
    for(auto i = elements.Begin(); i != elements.End(); ++i) {
      if(auto& action = std::get<1>(*i)) {
        list.push_back({mapping.m_key_sequence, std::get<0>(*i), *action});
      }
    }
  }
  return list;
}

std::ostream& Spire::operator <<(std::ostream& out,
    const KeyBindings::OrderAction& action) {
  out << '(' << action.m_name << ", " << action.m_type << ", " <<
    action.m_side << ", " << action.m_time_in_force << ", " <<
    action.m_quantity << ", ";
  return ::operator <<(out, action.m_tags) << ')';
}

std::ostream& Spire::operator <<(std::ostream& out,
    const KeyBindings::OrderAction::CustomTag& tag) {
  out << '(' << tag.m_name << ' ';
  std::visit(
    [&](auto& value) -> decltype(auto) {
      return out << value;
    }, tag.m_value);
  return out << ')';
}

std::ostream& Spire::operator <<(std::ostream& out,
    KeyBindings::CancelAction action) {
  switch(action) {
    case KeyBindings::CancelAction::MOST_RECENT:
      return out << "MOST_RECENT";
    case KeyBindings::CancelAction::MOST_RECENT_ASK:
      return out << "MOST_RECENT_ASK";
    case KeyBindings::CancelAction::MOST_RECENT_BID:
      return out << "MOST_RECENT_BID";
    case KeyBindings::CancelAction::OLDEST:
      return out << "OLDEST";
    case KeyBindings::CancelAction::OLDEST_ASK:
      return out << "OLDEST_ASK";
    case KeyBindings::CancelAction::OLDEST_BID:
      return out << "OLDEST_BID";
    case KeyBindings::CancelAction::ALL:
      return out << "ALL";
    case KeyBindings::CancelAction::ALL_ASKS:
      return out << "ALL_ASKS";
    case KeyBindings::CancelAction::ALL_BIDS:
      return out << "ALL_BIDS";
    case KeyBindings::CancelAction::CLOSEST_ASK:
      return out << "CLOSEST_ASK";
    case KeyBindings::CancelAction::CLOSEST_BID:
      return out << "CLOSEST_BID";
    case KeyBindings::CancelAction::FURTHEST_ASK:
      return out << "FURTHEST_ASK";
    case KeyBindings::CancelAction::FURTHEST_BID:
      return out << "FURTHEST_BID";
    default:
      return out << "?";
  }
}
