#include "Spire/Ui/SecurityStack.hpp"

using namespace Nexus;
using namespace Spire;

void SecurityStack::push(const Nexus::Security& security) {
  if(security == Security()) {
    return;
  }
  if(!m_securities.empty() && m_securities.front() == security) {
    return;
  }
  m_securities.push_front(security);
  if(m_securities.size() > 1 && m_securities.back() == m_securities.front()) {
    m_securities.pop_back();
  }
}

Security SecurityStack::push_front(const Security& security) {
  while(!m_securities.empty()) {
    auto top = std::move(m_securities.front());
    m_securities.pop_front();
    if(top != security) {
      m_securities.push_back(security);
      return top;
    }
  }
  return {};
}

Security SecurityStack::push_back(const Security& security) {
  while(!m_securities.empty()) {
    auto bottom = std::move(m_securities.back());
    m_securities.pop_back();
    if(bottom != security) {
      m_securities.push_front(security);
      return bottom;
    }
  }
  return {};
}
