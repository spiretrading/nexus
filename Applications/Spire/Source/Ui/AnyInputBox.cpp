#include "Spire/Ui/AnyInputBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const std::shared_ptr<AnyValueModel>& AnyInputBox::get_current() const {
  throw std::runtime_error("Not implemented.");
}

const AnyRef& AnyInputBox::get_submission() const {
  throw std::runtime_error("Not implemented.");
}

bool AnyInputBox::is_read_only() const {
  return false;
}

void AnyInputBox::set_read_only(bool read_only) {
}

connection AnyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return {};
}

connection AnyInputBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return {};
}
