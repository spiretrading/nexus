#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;

OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::
    OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings(
    const OrderImbalanceIndicatorMarketSelectionWidget& widget) {}

QWidget* OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::Reopen(
    RefType<UserProfile> userProfile) const {

  // TODO
  return nullptr;
}

void OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::Apply(
    RefType<UserProfile> userProfile, Out<QWidget> widget) const {

  // TODO
}

OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::
    OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings() {}
