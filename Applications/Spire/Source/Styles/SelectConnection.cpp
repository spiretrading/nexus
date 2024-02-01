#include "Spire/Styles/SelectConnection.hpp"

using namespace Spire;
using namespace Spire::Styles;

bool SelectConnection::is_connected() const {
  return m_state != nullptr;
}
