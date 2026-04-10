#include "Spire/Spire/TickerDeck.hpp"
#include <algorithm>

using namespace boost;
using namespace Nexus;
using namespace Spire;

optional<Ticker> TickerDeck::get_top() const {
  if(m_deck.empty()) {
    return none;
  }
  return m_deck.front();
}

void TickerDeck::add(const Ticker& ticker) {
  if(!ticker) {
    return;
  }
  auto i = std::find(m_deck.begin(), m_deck.end(), ticker);
  if(i != m_deck.end()) {
    if(i == m_deck.begin()) {
      return;
    }
    m_deck.erase(i);
  }
  m_deck.push_front(ticker);
}

optional<Ticker> TickerDeck::rotate_bottom() {
  if(m_deck.size() <= 1) {
    return none;
  }
  auto ticker = m_deck.back();
  m_deck.pop_back();
  m_deck.push_front(ticker);
  return m_deck.front();
}

optional<Ticker> TickerDeck::rotate_top() {
  if(m_deck.size() <= 1) {
    return none;
  }
  auto ticker = m_deck.front();
  m_deck.pop_front();
  m_deck.push_back(ticker);
  return m_deck.front();
}
