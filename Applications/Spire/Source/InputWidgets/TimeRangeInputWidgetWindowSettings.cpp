#include "Spire/InputWidgets/TimeRangeInputWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;

TimeRangeInputWidgetWindowSettings::TimeRangeInputWidgetWindowSettings(
    const TimeRangeInputWidget& widget) {}

QWidget* TimeRangeInputWidgetWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {

  // TODO
  return nullptr;
}

void TimeRangeInputWidgetWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {

  // TODO
}

TimeRangeInputWidgetWindowSettings::TimeRangeInputWidgetWindowSettings() {}
