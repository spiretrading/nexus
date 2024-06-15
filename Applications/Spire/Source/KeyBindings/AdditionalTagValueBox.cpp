#include "Spire/KeyBindings/AdditionalTagValueBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

AdditionalTagValueBox::AdditionalTagValueBox(
  std::shared_ptr<AdditionalTagValueModel> current,
  std::shared_ptr<AdditionalTagSchemaModel> schema, QWidget* parent)
  : m_current(std::move(current)) {}

const std::shared_ptr<AdditionalTagValueModel>&
    AdditionalTagValueBox::get_current() const {
  return m_current;
}

bool AdditionalTagValueBox::is_read_only() const {
  return false;
}

void AdditionalTagValueBox::set_read_only(bool is_read_only) {}

connection AdditionalTagValueBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
