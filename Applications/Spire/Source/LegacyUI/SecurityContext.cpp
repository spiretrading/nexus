#include "Spire/LegacyUI/SecurityContext.hpp"
#include <unordered_map>
#include <Beam/ServiceLocator/SessionEncryption.hpp>

using namespace Beam;
using namespace boost;
using namespace boost::container;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  std::unordered_map<std::string, SecurityContext*> contexts;

  std::string GenerateUniqueIdentifier() {
    std::string identifier;
    do {
      identifier = generate_session_id();
    } while(contexts.find(identifier) != contexts.end());
    return identifier;
  }
}

boost::optional<SecurityContext&> SecurityContext::FindSecurityContext(
    const std::string& identifier) {
  auto contextIterator = contexts.find(identifier);
  if(contextIterator == contexts.end()) {
    return none;
  }
  return *contextIterator->second;
}

SecurityContext::~SecurityContext() {
  if(m_outgoingLink != nullptr) {
    m_outgoingLink->m_incomingLinks.erase(this);
  }
  std::set<SecurityContext*> incomingLinks = m_incomingLinks;
  for(SecurityContext* incomingLink : incomingLinks) {
    if(m_outgoingLink == nullptr) {
      incomingLink->Unlink();
    } else {
      incomingLink->Link(*m_outgoingLink);
    }
  }
  contexts.erase(m_identifier);
}

void SecurityContext::Link(SecurityContext& context) {
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

void SecurityContext::Unlink() {
  if(m_outgoingLink == nullptr) {
    return;
  }
  m_outgoingLink->m_incomingLinks.erase(this);
  m_outgoingLink = nullptr;
  HandleUnlink();
}

const Security& SecurityContext::GetDisplayedSecurity() const {
  return m_security;
}

const std::string& SecurityContext::GetIdentifier() const {
  return m_identifier;
}

const std::string& SecurityContext::GetLinkedIdentifier() const {
  static const std::string NONE_IDENTIFIER;
  if(m_outgoingLink == nullptr) {
    return NONE_IDENTIFIER;
  }
  return m_outgoingLink->GetIdentifier();
}

connection SecurityContext::ConnectSecurityDisplaySignal(
    const SecurityDisplaySignal::slot_type& slot) const {
  return m_securityDisplaySignal.connect(slot);
}

SecurityContext::SecurityContext()
    : m_identifier(GenerateUniqueIdentifier()),
      m_outgoingLink(nullptr) {}

SecurityContext::SecurityContext(const std::string& identifier)
    : m_identifier(identifier),
      m_outgoingLink(nullptr) {
  if(m_identifier.empty()) {
    m_identifier = GenerateUniqueIdentifier();
  }
  contexts[m_identifier] = this;
}

void SecurityContext::HandleLink(SecurityContext& context) {}

void SecurityContext::HandleUnlink() {}

void SecurityContext::SetDisplayedSecurity(const Nexus::Security& security) {
  if(m_security == security) {
    return;
  }
  m_security = security;
  m_securityDisplaySignal(m_security);
}
