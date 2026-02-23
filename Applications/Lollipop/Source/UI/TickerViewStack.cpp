#include "Spire/UI/TickerViewStack.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

TickerViewStack::TickerViewStack() {}

void TickerViewStack::Push(const Ticker& ticker) {
  if(ticker == Ticker()) {
    return;
  }
  m_tickers.push_front(ticker);
}

void TickerViewStack::PushUp(const Ticker& ticker,
    const std::function<void (const Ticker&)>& continuation) {
  if(m_tickers.empty()) {
    return;
  }
  Ticker topTicker = m_tickers.front();
  m_tickers.pop_front();
  m_tickers.push_back(ticker);
  continuation(topTicker);
}

void TickerViewStack::PushDown(const Ticker& ticker,
    const std::function<void (const Ticker&)>& continuation) {
  if(m_tickers.empty()) {
    return;
  }
  Ticker bottomTicker = m_tickers.back();
  m_tickers.pop_back();
  m_tickers.push_front(ticker);
  continuation(bottomTicker);
}
