#ifndef SPIRE_KEY_FILTER_PANEL_HPP
#define SPIRE_KEY_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/TagBox.hpp"

namespace Spire {
  template<>
  struct OpenFilterPanelAdaptor<AnyTagBox> {
    using SubmissionType = std::shared_ptr<AnyListModel>;

    static bool is_empty(AnyTagBox& tag_box);
    static void clear(AnyTagBox& tag_box);
    static SubmissionType get_current(AnyTagBox& tag_box);
    static boost::signals2::connection connect_current(
      AnyTagBox& tag_box, const std::function<void()>& slot);
  };

  extern template class OpenFilterPanel<AnyTagBox>;

  /** An OpenFilterPanel specialized for an AnyTagBox. */
  using KeyFilterPanel = OpenFilterPanel<AnyTagBox>;
}

#endif
