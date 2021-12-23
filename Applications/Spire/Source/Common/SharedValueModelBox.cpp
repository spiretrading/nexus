#include "Spire/Spire/SharedValueModelBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const std::shared_ptr<void>& SharedValueModelBox::get_model() const {
  return m_model;
}

void SharedValueModelBox::signal_update() {
  m_signal_update();
}

connection SharedValueModelBox::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_connect_update_signal(slot);
}
