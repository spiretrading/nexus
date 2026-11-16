#include "Spire/Blotter/LegacyBlotterWindowSettings.hpp"

using namespace Beam;
using namespace Spire;

std::string LegacyBlotterWindowSettings::GetName() const {
  return "Blotter - " + m_blotter_name;
}

QWidget* LegacyBlotterWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  // TODO: Restore legacy blotter windows using the new blotter.
  return nullptr;
}

void LegacyBlotterWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  // TODO: Apply compatible legacy settings to the new blotter.
}
