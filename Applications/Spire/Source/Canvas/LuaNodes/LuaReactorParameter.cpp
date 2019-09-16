#include "Spire/Canvas/LuaNodes/LuaReactorParameter.hpp"

using namespace Aspen;
using namespace Spire;

const Box<void>& LuaReactorParameter::GetReactor() const {
  return m_reactor;
}

LuaReactorParameter::LuaReactorParameter(Box<void> reactor)
  : m_reactor(std::move(reactor)) {}
