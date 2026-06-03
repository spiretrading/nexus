#ifndef SPIRE_KEY_FILTER_PANEL_HPP
#define SPIRE_KEY_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/TagBox.hpp"

namespace Spire {
namespace Details {
  template<>
  struct TagComboBoxTraits<AnyTagBox> {
    using SubmissionType = std::shared_ptr<AnyListModel>;

    static bool is_empty(AnyTagBox& box);
    static void clear(AnyTagBox& box);
    static SubmissionType get_current(AnyTagBox& box);
    static boost::signals2::connection connect_current(
      AnyTagBox& box, const std::function<void()>& slot);
  };
}

  extern template class OpenFilterPanel<AnyTagBox>;

  /** An OpenFilterPanel specialized for an AnyTagBox. */
  using KeyFilterPanel = OpenFilterPanel<AnyTagBox>;
}

#endif
