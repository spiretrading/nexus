#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire::Details {
  bool TagComboBoxTraits<AnyTagBox>::is_empty(AnyTagBox& box) {
    return box.get_tags()->get_size() == 0;
  }

  void TagComboBoxTraits<AnyTagBox>::clear(AnyTagBox& box) {
    Spire::clear(*box.get_tags());
  }

  TagComboBoxTraits<AnyTagBox>::SubmissionType
      TagComboBoxTraits<AnyTagBox>::get_current(AnyTagBox& box) {
    return box.get_tags();
  }

  boost::signals2::connection
      TagComboBoxTraits<AnyTagBox>::connect_current(
        AnyTagBox& box, const std::function<void()>& slot) {
    return box.get_tags()->connect_operation_signal(
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
}

template class Spire::OpenFilterPanel<Spire::AnyTagBox>;
