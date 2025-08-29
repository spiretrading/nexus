#ifndef SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGETWINDOWSETTINGS_HPP
#define SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGETWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings
      \brief Stores the window settings for an
             OrderImbalanceIndicatorVenueSelectionWidget.
   */
  class OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings :
      public UI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;

      OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings::Shuttle(
    Shuttler& shuttle, unsigned int version) {}
}

#endif
