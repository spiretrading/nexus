#include "Spire/LegacyUI/SecurityViewStack.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

SecurityViewStack::SecurityViewStack() {}

void SecurityViewStack::Push(const Security& security) {
  if(!security) {
    return;
  }
  m_securities.push_front(security);
}

void SecurityViewStack::PushUp(const Security& security,
    const std::function<void (const Security&)>& continuation) {
  if(m_securities.empty()) {
    return;
  }
  Security topSecurity = m_securities.front();
  m_securities.pop_front();
  m_securities.push_back(security);
  continuation(topSecurity);
}

void SecurityViewStack::PushDown(const Security& security,
    const std::function<void (const Security&)>& continuation) {
  if(m_securities.empty()) {
    return;
  }
  Security bottomSecurity = m_securities.back();
  m_securities.pop_back();
  m_securities.push_front(security);
  continuation(bottomSecurity);
}
