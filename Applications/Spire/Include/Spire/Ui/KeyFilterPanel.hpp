#ifndef SPIRE_KEY_FILTER_PANEL_HPP
#define SPIRE_KEY_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/TagBox.hpp"

namespace Spire {

  /** An OpenFilterPanelAdaptor specialized for an AnyTagBox. */
  template<>
  struct OpenFilterPanelAdaptor<AnyTagBox> {

    /** The type of the input's current value. */
    using Type = std::shared_ptr<AnyListModel>;

    /** Returns whether the input contains no values. */
    static bool is_empty(AnyTagBox& tag_box);

    /** Removes all values from the input. */
    static void clear(AnyTagBox& tag_box);

    /** Returns the current value of the input. */
    static Type get_current(AnyTagBox& tag_box);

    /** Connects a slot called when the input's current value changes. */
    static boost::signals2::connection connect_current(
      AnyTagBox& tag_box, const std::function<void()>& slot);
  };

  extern template class OpenFilterPanel<AnyTagBox>;

  /** An OpenFilterPanel specialized for an AnyTagBox. */
  using KeyFilterPanel = OpenFilterPanel<AnyTagBox>;
}

#endif
