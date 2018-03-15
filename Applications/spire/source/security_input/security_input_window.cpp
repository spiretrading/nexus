#include "spire/security_input/security_input_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

security_input_window::security_input_window(security_input_model& model) {}

connection security_input_window::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return {};
}
