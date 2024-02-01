#include "Spire/Ui/AnyInputBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const std::shared_ptr<AnyValueModel>& AnyInputBox::get_current() const {
  return m_input_box->get_current();
}

const std::shared_ptr<HighlightModel>& AnyInputBox::get_highlight() const {
  return m_input_box->get_highlight();
}

const AnyRef& AnyInputBox::get_submission() const {
  return m_input_box->get_submission();
}

void AnyInputBox::set_placeholder(const QString& placeholder) {
  m_input_box->set_placeholder(placeholder);
}

bool AnyInputBox::is_read_only() const {
  return m_input_box->is_read_only();
}

void AnyInputBox::set_read_only(bool read_only) {
  return m_input_box->set_read_only(read_only);
}

connection AnyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_input_box->connect_submit_signal(slot);
}

connection AnyInputBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_input_box->connect_reject_signal(slot);
}
