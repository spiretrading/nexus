#include "Spire/Ui/TickerStack.hpp"

using namespace Nexus;
using namespace Spire;

void TickerStack::push(const Nexus::Ticker& ticker) {
  if(!ticker) {
    return;
  }
  if(!m_tickers.empty() && m_tickers.front() == ticker) {
    return;
  }
  m_tickers.push_front(ticker);
  if(m_tickers.size() > 1 && m_tickers.back() == m_tickers.front()) {
    m_tickers.pop_back();
  }
}

Ticker TickerStack::push_front(const Ticker& ticker) {
  while(!m_tickers.empty()) {
    auto top = std::move(m_tickers.front());
    m_tickers.pop_front();
    if(top != ticker) {
      m_tickers.push_back(ticker);
      return top;
    }
  }
  return {};
}

Ticker TickerStack::push_back(const Ticker& ticker) {
  while(!m_tickers.empty()) {
    auto bottom = std::move(m_tickers.back());
    m_tickers.pop_back();
    if(bottom != ticker) {
      m_tickers.push_front(ticker);
      return bottom;
    }
  }
  return {};
}
