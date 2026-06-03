#ifndef SPIRE_SCOPE_FILTER_PANEL_HPP
#define SPIRE_SCOPE_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/ScopeBox.hpp"

namespace Spire {

  /** An OpenFilterPanelAdaptor specialized for a ScopeBox. */
  template<>
  struct OpenFilterPanelAdaptor<ScopeBox> {

    /** The type of the input's current value. */
    using Type = Nexus::Scope;

    /** Returns whether the input contains no values. */
    static bool is_empty(ScopeBox& scope_box);

    /** Removes all values from the input. */
    static void clear(ScopeBox& scope_box);

    /** Returns the current value of the input. */
    static Type get_current(ScopeBox& scope_box);

    /** Connects a slot called when the input's current value changes. */
    static boost::signals2::connection connect_current(
      ScopeBox& scope_box, const std::function<void()>& slot);
  };

  extern template class OpenFilterPanel<ScopeBox>;

  /** An OpenFilterPanel specialized for a ScopeBox. */
  using ScopeFilterPanel = OpenFilterPanel<ScopeBox>;
}

#endif
