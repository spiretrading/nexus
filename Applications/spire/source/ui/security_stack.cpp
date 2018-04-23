#include "spire/ui/security_stack.hpp"

using namespace Nexus;
using namespace spire;

void security_stack::push(Nexus::Security s) {
  if(s == Security()) {
    return;
  }
  if(!m_securities.empty() && m_securities.front() == s) {
    return;
  }
  m_securities.push_front(std::move(s));
  if(m_securities.size() > 1 && m_securities.back() == m_securities.front()) {
    m_securities.pop_back();
  }
}

Security security_stack::push_front(Security s) {
  while(!m_securities.empty()) {
    auto top = std::move(m_securities.front());
    m_securities.pop_front();
    if(top != s) {
      m_securities.push_back(std::move(s));
      return top;
    }
  }
  return {};
}

Security security_stack::push_back(Security s) {
  while(!m_securities.empty()) {
    auto bottom = std::move(m_securities.back());
    m_securities.pop_back();
    if(bottom != s) {
      m_securities.push_front(std::move(s));
      return bottom;
    }
  }
  return {};
}
