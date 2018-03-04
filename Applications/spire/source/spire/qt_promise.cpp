#include "spire/spire/qt_promise.hpp"

using namespace spire;

qt_promise::qt_promise(qt_promise&& other)
    : m_imp(std::move(other.m_imp)) {}

qt_promise::~qt_promise() {
  disconnect();
}

void qt_promise::disconnect() {
  if(m_imp == nullptr) {
    return;
  }
  m_imp->disconnect();
  m_imp.reset();
}

qt_promise& qt_promise::operator =(qt_promise&& other) {
  disconnect();
  m_imp = std::move(other.m_imp);
  return *this;
}
