#include "Spire/LegacyUI/CanvasWindowSettings.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/LegacyUI/CanvasWindow.hpp"
#include "ui_CanvasWindow.h"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

CanvasWindowSettings::CanvasWindowSettings() {}

CanvasWindowSettings::CanvasWindowSettings(const CanvasWindow& window)
    : m_geometry(window.saveGeometry()) {
  auto roots = window.m_ui->m_canvasTable->GetRoots();
  for(const auto& root : roots) {
    auto node = make_tuple(window.m_ui->m_canvasTable->GetCoordinate(*root),
      CanvasNode::Clone(*root));
    m_nodes.emplace_back(move(node));
  }
}

string CanvasWindowSettings::GetName() const {
  return "Canvas";
}

QWidget* CanvasWindowSettings::Reopen(Ref<UserProfile> userProfile) const {
  auto window = new CanvasWindow(Ref(userProfile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), out(*window));
  return window;
}

void CanvasWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  auto& window = dynamic_cast<CanvasWindow&>(*widget);
  restore_geometry(window, m_geometry);
  for(const auto& node : m_nodes) {
    window.m_ui->m_canvasTable->Add(get<0>(node), *get<1>(node));
  }
}
