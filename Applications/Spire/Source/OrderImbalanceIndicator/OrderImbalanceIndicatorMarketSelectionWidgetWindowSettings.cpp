#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;

OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::
    OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings(
    const OrderImbalanceIndicatorMarketSelectionWidget& widget) {}

QWidget* OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {

  // TODO
  return nullptr;
}

void OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {

  // TODO
}

OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::
    OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings() {}
