#include "Spire/UI/ToolbarWindowSettings.hpp"
#include "Spire/UI/Toolbar.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

ToolbarWindowSettings::ToolbarWindowSettings()
    : m_position(0, 0) {}

ToolbarWindowSettings::ToolbarWindowSettings(const Toolbar& toolbar)
    : m_position(toolbar.pos()) {}

ToolbarWindowSettings::~ToolbarWindowSettings() {}

string ToolbarWindowSettings::GetName() const {
  return "Toolbar";
}

QWidget* ToolbarWindowSettings::Reopen(Ref<UserProfile> profile) const {
  Toolbar* toolbar = new Toolbar(Ref(profile));
  Apply(Ref(profile), Store(*toolbar));
  return toolbar;
}

void ToolbarWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  Toolbar& window = dynamic_cast<Toolbar&>(*widget);
  window.move(m_position);
}
