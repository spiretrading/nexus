#ifndef SPIRE_SCOPE_FILTER_PANEL_HPP
#define SPIRE_SCOPE_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/ScopeBox.hpp"

namespace Spire {

  template<>
  struct OpenFilterPanelAdaptor<ScopeBox> {
    using Type = Nexus::Scope;

    static bool is_empty(ScopeBox& scope_box);
    static void clear(ScopeBox& scope_box);
    static Type get_current(ScopeBox& scope_box);
    static boost::signals2::connection connect_current(
      ScopeBox& scope_box, const std::function<void()>& slot);
  };

  extern template class OpenFilterPanel<ScopeBox>;

  /** An OpenFilterPanel specialized for a ScopeBox. */
  using ScopeFilterPanel = OpenFilterPanel<ScopeBox>;
}

#endif
