#include "spire/security_input/security_input_box.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_box::security_input_box(security_input_model& model)
    : m_model(&model) {}

connection security_input_box::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}
