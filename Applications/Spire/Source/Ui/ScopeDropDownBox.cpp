#include "Spire/Ui/ScopeDropDownBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ScopeListItem.hpp"

using namespace Nexus;
using namespace Spire;

ScopeDropDownBox* Spire::make_scope_drop_down_box(
    std::shared_ptr<ScopeListModel> scopes, QWidget* parent) {
  return make_scope_drop_down_box(std::move(scopes),
    std::make_shared<LocalScopeModel>(), parent);
}

ScopeDropDownBox* Spire::make_scope_drop_down_box(
    std::shared_ptr<ScopeListModel> scopes,
    std::shared_ptr<ScopeModel> current, QWidget* parent) {
  auto settings = ScopeDropDownBox::Settings();
  settings.m_cases = std::move(scopes);
  settings.m_current = std::move(current);
  settings.m_view_builder = [] (const auto& scope) {
    return new ScopeListItem(scope);
  };
  return new ScopeDropDownBox(std::move(settings), parent);
}
