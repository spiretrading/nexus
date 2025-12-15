#ifndef SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGETWINDOWSETTINGS_HPP
#define SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGETWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"

namespace Spire {

  /*! \class OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings
      \brief Stores the window settings for an
             OrderImbalanceIndicatorVenueSelectionWidget.
   */
  class OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings :
      public LegacyUI::WindowSettings {
    public:

      //! Constructs an
      //! OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings.
      /*!
        \param widget The widget to represent.
      */
      OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings(
        const OrderImbalanceIndicatorVenueSelectionWidget& widget);

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;

      OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings();
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings::shuttle(
    S& shuttle, unsigned int version) {}
}

#endif
