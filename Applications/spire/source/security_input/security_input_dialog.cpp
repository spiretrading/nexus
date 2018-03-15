#include "spire/security_input/security_input_dialog.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_dialog::security_input_dialog(security_input_model& model) {}

const Security& security_input_dialog::get_security() const noexcept {
  return m_security;
}
