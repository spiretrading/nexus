#include "WebPortal/RiskPortfolioSubscription.hpp"

using namespace Beam;
using namespace Nexus;

RiskPortfolioSubscription::RiskPortfolioSubscription(
    const RiskPortfolioUpdatePublisher& publisher)
    : m_updates(std::make_shared<Queue<RiskInventoryEntry>>()) {
  publisher.with([&] {
    publisher.monitor(m_updates);
    while(auto entry = m_updates->try_pop()) {
      m_snapshot.push_back(*entry);
    }
  });
  if(m_updates->is_broken()) {
    m_updates->pop();
  }
}

RiskPortfolioSubscription::~RiskPortfolioSubscription() {
  close();
}

const std::vector<RiskInventoryEntry>&
    RiskPortfolioSubscription::get_snapshot() const {
  return m_snapshot;
}

RiskInventoryEntry RiskPortfolioSubscription::pop() {
  return m_updates->pop();
}

void RiskPortfolioSubscription::close() {
  m_updates->close();
}
