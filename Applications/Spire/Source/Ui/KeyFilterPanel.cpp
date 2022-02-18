#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/KeyInputBox.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  AnyInputBox* input_box_builder(std::shared_ptr<KeySequenceValueModel> current,
      std::shared_ptr<AnyListModel> matches) {
    auto input_box = new AnyInputBox(*(new KeyInputBox(std::move(current))));
    input_box->get_current()->connect_update_signal([=] (const auto& current) {
      auto sequence = any_cast<QKeySequence>(current);
      if(sequence.isEmpty()) {
        return;
      }
      for(auto i = 0; i < matches->get_size(); ++i) {
        if(sequence == std::any_cast<QKeySequence>(matches->get(i))) {
          input_box->get_current()->set(QKeySequence());
          break;
        }
      }
    });
    input_box->connect_submit_signal([=] (const auto& submission) {
      if(!any_cast<QKeySequence>(submission).isEmpty()) {
        input_box->get_current()->set(QKeySequence());
        matches->push(any_cast<QKeySequence>(submission));
      }
    });
    return input_box;
  }
}

KeyFilterPanel* Spire::make_key_filter_panel(QWidget& parent) {
  return make_key_filter_panel(std::make_shared<LocalKeySequenceValueModel>(),
    parent);
}

KeyFilterPanel* Spire::make_key_filter_panel(
    std::shared_ptr<KeySequenceValueModel> current, QWidget& parent) {
  return make_key_filter_panel(std::move(current),
    std::make_shared<ArrayListModel<QKeySequence>>(),
    std::make_shared<LocalValueModel<KeyFilterPanel::Mode>>(
      KeyFilterPanel::Mode::INCLUDE), parent);
}

KeyFilterPanel* Spire::make_key_filter_panel(
  std::shared_ptr<KeySequenceValueModel> current,
  std::shared_ptr<ListModel<QKeySequence>> matches,
  std::shared_ptr<ValueModel<KeyFilterPanel::Mode>> mode, QWidget& parent) {
  return new KeyFilterPanel(
    std::bind_front(input_box_builder, std::move(current)),
    std::move(matches), std::move(mode), QObject::tr("Filter by Key"), parent);
}
