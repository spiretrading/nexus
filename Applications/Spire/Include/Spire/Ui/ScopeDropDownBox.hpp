#ifndef SPIRE_SCOPE_DROP_DOWN_BOX_HPP
#define SPIRE_SCOPE_DROP_DOWN_BOX_HPP
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::Scope. */
  using ScopeModel = ValueModel<Nexus::Scope>;

  /** A LocalValueModel over a Nexus::Scope. */
  using LocalScopeModel = LocalValueModel<Nexus::Scope>;

  /** A ListModel over Nexus::Scope. */
  using ScopeListModel = ListModel<Nexus::Scope>;

  /** An EnumBox specialized for a Nexus::Scope. */
  using ScopeDropDownBox = EnumBox<Nexus::Scope>;

  /**
   * Returns a new ScopeDropDownBox using a LocalScopeModel.
   * @param scopes A closed set of scopes.
   * @param parent The parent widget.
   */
  ScopeDropDownBox* make_scope_drop_down_box(
    std::shared_ptr<ScopeListModel> scopes, QWidget* parent = nullptr);

  /**
   * Returns a ScopeDropDownBox.
   * @param scopes A closed set of scopes.
   * @param model The current value model.
   * @param parent The parent widget.
   */
   ScopeDropDownBox* make_scope_drop_down_box(
    std::shared_ptr<ScopeListModel> scopes,
    std::shared_ptr<ScopeModel> current, QWidget* parent = nullptr);
}

#endif
