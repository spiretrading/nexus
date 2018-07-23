#include "Spire/InputWidgets/TimeRangeInputWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;

TimeRangeInputWidgetWindowSettings::TimeRangeInputWidgetWindowSettings(
    const TimeRangeInputWidget& widget) {}

QWidget* TimeRangeInputWidgetWindowSettings::Reopen(
    RefType<UserProfile> userProfile) const {

  // TODO
  return nullptr;
}

void TimeRangeInputWidgetWindowSettings::Apply(RefType<UserProfile> userProfile,
    Out<QWidget> widget) const {

  // TODO
}

TimeRangeInputWidgetWindowSettings::TimeRangeInputWidgetWindowSettings() {}
