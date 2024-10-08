#include "Spire/Spire/SecurityDeck.hpp"
#include <algorithm>

using namespace boost;
using namespace Nexus;
using namespace Spire;

void SecurityDeck::add(const Security& security) {
  if(security == Security()) {
    return;
  }
  auto i = std::find(m_deck.begin(), m_deck.end(), security);
  if(i != m_deck.end()) {
    if(i == m_deck.begin()) {
      return;
    }
    m_deck.erase(i);
  }
  m_deck.push_front(security);
}

optional<Security> SecurityDeck::rotate_bottom() {
  if(m_deck.size() <= 1) {
    return none;
  }
  auto security = m_deck.back();
  m_deck.pop_back();
  m_deck.push_front(security);
  return m_deck.front();
}

optional<Security> SecurityDeck::rotate_top() {
  if(m_deck.size() <= 1) {
    return none;
  }
  auto security = m_deck.front();
  m_deck.pop_front();
  m_deck.push_back(security);
  return m_deck.front();
}
