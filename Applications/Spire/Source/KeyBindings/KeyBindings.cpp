#include "Spire/KeyBindings/KeyBindings.hpp"
#include <algorithm>
#include <type_traits>
#include <variant>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/optional/optional_io.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int KeyBindings::Tag::get_key() const {
  return m_key;
}

KeyBindings::Tag::Type KeyBindings::Tag::get_value() const {
  return m_value;
}

bool KeyBindings::Tag::operator ==(const Tag& other) const {
  return m_key == other.m_key && m_value == other.m_value;
}

bool KeyBindings::Tag::operator !=(const Tag& other) const {
  return !(*this == other);
}

KeyBindings::OrderAction::OrderAction(std::string name, std::vector<Tag> tags)
  : m_name(std::move(name)),
    m_tags(std::move(tags)) {}

const std::string& KeyBindings::OrderAction::get_name() const {
  return m_name;
}

void KeyBindings::OrderAction::set_name(std::string name) {
  m_name = std::move(name);
}

boost::optional<const KeyBindings::Tag&> KeyBindings::OrderAction::get_tag(
    int tag_key) const {
  auto it = std::find_if(m_tags.begin(), m_tags.end(), [&] (auto& stored_tag) {
    return stored_tag.get_key() == stored_tag.get_key();
  });
  if(it != m_tags.end()) {
    return *it;
  } else {
    return boost::none;
  }
}

void KeyBindings::OrderAction::set_tag(Tag tag) {
  auto it = std::find_if(m_tags.begin(), m_tags.end(), [&] (auto& stored_tag) {
    return stored_tag.get_key() == stored_tag.get_key();
  });
  if(it != m_tags.end()) {
    *it = std::move(tag);
  } else {
    m_tags.push_back(std::move(tag));
  }
}

const std::vector<KeyBindings::Tag>& KeyBindings::OrderAction::get_tags()
    const {
  return m_tags;
}

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
  auto action1 = KeyBindings::OrderAction{"", {Tag(40, OrderType(
    OrderType::LIMIT)), Tag(54, Side(Side::BID)), Tag(59,
    boost::optional<TimeInForce>()), Tag(38, 100)}};
  bindings.set(Qt::Key_F5, region, action1);
  auto action2 = KeyBindings::OrderAction{"", {Tag(40, OrderType(
    OrderType::LIMIT)), Tag(54, Side(Side::ASK)), Tag(59,
    boost::optional<TimeInForce>()), Tag(38, 100)}};
  bindings.set(Qt::Key_F6, region, action2);
  auto action3 = KeyBindings::OrderAction{"", {Tag(40, OrderType(
    OrderType::MARKET)), Tag(54, Side(Side::BID)), Tag(59,
    boost::optional<TimeInForce>()), Tag(38, 100)}};
  bindings.set(Qt::Key_F7, region, action3);
  auto action4 = KeyBindings::OrderAction{"", {Tag(40, OrderType(
    OrderType::MARKET)), Tag(54, Side(Side::ASK)), Tag(59,
    boost::optional<TimeInForce>()), Tag(38, 100)}};
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
    auto binding = KeyBindingMapping{sequence, Actions(boost::none)};
    binding.m_actions.Set(region, action);
    m_bindings.push_back(std::move(binding));
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
    const boost::optional<Region>& region) {
  if(!region) {
    return out << "(None)";
  }
  out << '(' << region->GetName() << " (";
  for(auto country_code : region->GetCountries()) {
    out << ", " << country_code;
  }
  out << ") (";
  for(auto security : region->GetSecurities()) {
    out << ", " << security;
  }
  return out;
}

std::ostream& Spire::operator <<(std::ostream& out,
    const KeyBindings::Action& action) {
  out << '(';
  std::visit(
    [&] (auto& value) -> decltype(auto) {
      if constexpr(std::is_same_v<std::decay_t<decltype(value)>,
          KeyBindings::OrderAction>) {
        out << "ORDER ";
      } else {
        out << "CANCEL ";
      }
    }, action);
  return out << ')';
}

std::ostream& Spire::operator <<(std::ostream& out,
    const KeyBindings::OrderAction& action) {
  out << '(' << action.get_name();
  for(auto& tag : action.get_tags()) {
    out << ", " << tag;
  }
  return out << ')';
}

std::ostream& Spire::operator <<(std::ostream& out,
    const KeyBindings::Tag& tag) {
  out << '(' << tag.m_key << ' ';
  std::visit(
    [&] (auto& value) -> decltype(auto) {
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
