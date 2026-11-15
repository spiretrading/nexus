#include "WebPortal/BboQuoteSubscription.hpp"

using namespace Beam;
using namespace Nexus;

BboQuoteSubscription::BboQuoteSubscription(
    MarketDataClient& client, const TickerQuery& query)
    : m_ticker(query.get_index()),
      m_is_live(query.get_range().get_end() == Sequence::LAST),
      m_updates(std::make_shared<Queue<SequencedBboQuote>>()),
      m_is_closed(false) {
  client.query(query, m_updates);
}

BboQuoteSubscription::~BboQuoteSubscription() {
  close();
}

const Ticker& BboQuoteSubscription::get_ticker() const {
  return m_ticker;
}

std::vector<SequencedBboQuote> BboQuoteSubscription::get_snapshot() {
  auto snapshot = std::vector<SequencedBboQuote>();
  if(!m_is_live) {
    try {
      while(true) {
        snapshot.push_back(m_updates->pop());
      }
    } catch(const PipeBrokenException&) {
      return snapshot;
    }
  }
  while(auto value = m_updates->try_pop()) {
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
