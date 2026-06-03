#include "Spire/Ui/ScopeFilterPanel.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

bool OpenFilterPanelAdaptor<ScopeBox>::is_empty(ScopeBox& scope_box) {
  return scope_box.get_current()->get().is_empty();
}

void OpenFilterPanelAdaptor<ScopeBox>::clear(ScopeBox& scope_box) {
  scope_box.get_current()->set(Scope());
}

OpenFilterPanelAdaptor<ScopeBox>::Type
    OpenFilterPanelAdaptor<ScopeBox>::get_current(ScopeBox& scope_box) {
  return scope_box.get_current()->get();
}

connection OpenFilterPanelAdaptor<ScopeBox>::connect_current(
    ScopeBox& scope_box, const std::function<void()>& slot) {
  return scope_box.get_current()->connect_update_signal([=] (const auto&) {
    slot();
  });
}

template class OpenFilterPanel<ScopeBox>;
