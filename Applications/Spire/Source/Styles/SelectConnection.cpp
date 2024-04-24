#include "Spire/Styles/SelectConnection.hpp"

using namespace Spire;
using namespace Spire::Styles;

void SelectConnection::disconnect() {
  m_executor = nullptr;
}

bool SelectConnection::is_connected() const {
  return m_executor != nullptr && m_executor->is_connected();
}
