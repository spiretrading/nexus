#include "Spire/Spire/ModelTransactionLog.hpp"

using namespace Spire::Details;

ScopeExit::ScopeExit(std::function<void()> f)
  : m_f(std::move(f)) {}

ScopeExit::~ScopeExit() {
  m_f();
}
