#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Spire/ListModel.hpp"

using namespace boost::signals2;
using namespace Spire;

bool OpenFilterPanelAdaptor<AnyTagBox>::is_empty(AnyTagBox& tag_box) {
  return tag_box.get_tags()->get_size() == 0;
}

void OpenFilterPanelAdaptor<AnyTagBox>::clear(AnyTagBox& tag_box) {
  Spire::clear(*tag_box.get_tags());
}

OpenFilterPanelAdaptor<AnyTagBox>::SubmissionType
    OpenFilterPanelAdaptor<AnyTagBox>::get_current(AnyTagBox& tag_box) {
  return tag_box.get_tags();
}

connection OpenFilterPanelAdaptor<AnyTagBox>::connect_current(
    AnyTagBox& tag_box, const std::function<void()>& slot) {
  return tag_box.get_tags()->connect_operation_signal(
    [=] (const auto& operation) {
      visit(operation,
        [&] (const AnyListModel::AddOperation&) {
          slot();
        },
        [&] (const AnyListModel::RemoveOperation&) {
          slot();
        });
    });
}

template class OpenFilterPanel<AnyTagBox>;
