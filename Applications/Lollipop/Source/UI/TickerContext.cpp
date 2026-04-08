#include "Spire/UI/TickerContext.hpp"
#include <unordered_map>
#include <Beam/ServiceLocator/SessionEncryption.hpp>

using namespace Beam;
using namespace boost;
using namespace boost::container;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  std::unordered_map<std::string, TickerContext*> contexts;

  std::string GenerateUniqueIdentifier() {
    std::string identifier;
    do {
      identifier = generate_session_id();
    } while(contexts.find(identifier) != contexts.end());
    return identifier;
  }
}

boost::optional<TickerContext&> TickerContext::FindTickerContext(
    const std::string& identifier) {
  auto contextIterator = contexts.find(identifier);
  if(contextIterator == contexts.end()) {
    return none;
  }
  return *contextIterator->second;
}

TickerContext::~TickerContext() {
  if(m_outgoingLink != nullptr) {
    m_outgoingLink->m_incomingLinks.erase(this);
  }
  std::set<TickerContext*> incomingLinks = m_incomingLinks;
  for(TickerContext* incomingLink : incomingLinks) {
    if(m_outgoingLink == nullptr) {
      incomingLink->Unlink();
    } else {
      incomingLink->Link(*m_outgoingLink);
    }
  }
  contexts.erase(m_identifier);
}

void TickerContext::Link(TickerContext& context) {
  if(&context == m_outgoingLink) {
    return;
  }
  Unlink();
  if(&context == this) {
    return;
  }
  m_outgoingLink = &context;
  m_outgoingLink->m_incomingLinks.insert(this);
  HandleLink(context);
}

void TickerContext::Unlink() {
  if(m_outgoingLink == nullptr) {
    return;
  }
  m_outgoingLink->m_incomingLinks.erase(this);
  m_outgoingLink = nullptr;
  HandleUnlink();
}

const Ticker& TickerContext::GetDisplayedTicker() const {
  return m_ticker;
}

const std::string& TickerContext::GetIdentifier() const {
  return m_identifier;
}

const std::string& TickerContext::GetLinkedIdentifier() const {
  static const std::string NONE_IDENTIFIER;
  if(m_outgoingLink == nullptr) {
    return NONE_IDENTIFIER;
  }
  return m_outgoingLink->GetIdentifier();
}

connection TickerContext::ConnectTickerDisplaySignal(
    const TickerDisplaySignal::slot_type& slot) const {
  return m_tickerDisplaySignal.connect(slot);
}

TickerContext::TickerContext()
    : m_identifier(GenerateUniqueIdentifier()),
      m_outgoingLink(nullptr) {}

TickerContext::TickerContext(const std::string& identifier)
    : m_identifier(identifier),
      m_outgoingLink(nullptr) {
  if(m_identifier.empty()) {
    m_identifier = GenerateUniqueIdentifier();
  }
  contexts[m_identifier] = this;
}

void TickerContext::HandleLink(TickerContext& context) {}

void TickerContext::HandleUnlink() {}

void TickerContext::SetDisplayedTicker(const Nexus::Ticker& ticker) {
  if(m_ticker == ticker) {
    return;
  }
  m_ticker = ticker;
  m_tickerDisplaySignal(m_ticker);
}
