#include "Spire/Spire/ReferenceValueModelBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const std::shared_ptr<void>& ReferenceValueModelBox::get_model() const {
  return m_model;
}

void ReferenceValueModelBox::signal_update() {
  m_signal_update();
}

connection ReferenceValueModelBox::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_connect_update_signal(slot);
}
