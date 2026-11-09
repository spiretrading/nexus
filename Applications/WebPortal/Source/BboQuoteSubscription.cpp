#include "WebPortal/BboQuoteSubscription.hpp"

using namespace Beam;
using namespace Nexus;

BboQuoteSubscription::BboQuoteSubscription(
    MarketDataClient& client, Ticker ticker)
    : m_ticker(std::move(ticker)),
      m_updates(std::make_shared<Queue<SequencedBboQuote>>()),
      m_is_closed(false) {
  client.query(make_current_query(m_ticker), m_updates);
}

BboQuoteSubscription::~BboQuoteSubscription() {
  close();
}

const Ticker& BboQuoteSubscription::get_ticker() const {
  return m_ticker;
}

std::vector<SequencedBboQuote> BboQuoteSubscription::get_snapshot() {
  auto snapshot = std::vector<SequencedBboQuote>();
  if(auto value = m_updates->try_pop()) {
    snapshot.push_back(*value);
  }
  if(m_updates->is_broken()) {
    m_updates->pop();
  }
  return snapshot;
}

SequencedBboQuote BboQuoteSubscription::pop() {
  if(m_is_closed) {
    throw PipeBrokenException();
  }
  auto value = m_updates->pop();
  if(m_is_closed) {
    throw PipeBrokenException();
  }
  return value;
}

bool BboQuoteSubscription::is_closed() const {
  return m_is_closed;
}

void BboQuoteSubscription::close() {
  m_is_closed = true;
  m_updates->close();
}
