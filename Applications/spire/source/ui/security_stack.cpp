#include "spire/ui/security_stack.hpp"

using namespace Nexus;
using namespace spire;

void security_stack::push(Nexus::Security s) {
  if(s == Security()) {
    return;
  }
  m_securities.push_front(std::move(s));
}

Security security_stack::push_front(Security s) {
  if(m_securities.empty()) {
    return {};
  }
  auto top = std::move(m_securities.front());
  m_securities.pop_front();
  m_securities.push_back(std::move(s));
  return top;
}

Security security_stack::push_back(Security s) {
  if(m_securities.empty()) {
    return {};
  }
  auto bottom = std::move(m_securities.back());
  m_securities.pop_back();
  m_securities.push_front(std::move(s));
  return bottom;
}
