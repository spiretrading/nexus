#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Spire/ListModel.hpp"

using namespace boost::signals2;
using namespace Spire;

bool OpenFilterPanelAdaptor<KeyListBox>::is_empty(KeyListBox& key_list_box) {
  return key_list_box.get_keys()->get_size() == 0;
}

void OpenFilterPanelAdaptor<KeyListBox>::clear(KeyListBox& key_list_box) {
  Spire::clear(*key_list_box.get_keys());
}

OpenFilterPanelAdaptor<KeyListBox>::Type
    OpenFilterPanelAdaptor<KeyListBox>::get_current(KeyListBox& key_list_box) {
  return key_list_box.get_keys();
}

connection OpenFilterPanelAdaptor<KeyListBox>::connect_current(
    KeyListBox& key_list_box, const std::function<void()>& slot) {
  return key_list_box.get_keys()->connect_operation_signal(
    [=] (const auto& operation) {
      visit(operation,
        [&] (const KeySequenceListModel::AddOperation&) {
          slot();
        },
        [&] (const KeySequenceListModel::RemoveOperation&) {
          slot();
        });
    });
}

template class OpenFilterPanel<KeyListBox>;
