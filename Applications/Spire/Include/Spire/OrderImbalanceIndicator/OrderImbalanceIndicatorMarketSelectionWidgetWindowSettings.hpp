#ifndef SPIRE_ORDERIMBALANCEINDICATORMARKETSELECTIONWIDGETWINDOWSETTINGS_HPP
#define SPIRE_ORDERIMBALANCEINDICATORMARKETSELECTIONWIDGETWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"

namespace Spire {

  /*! \class OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings
      \brief Stores the window settings for an
             OrderImbalanceIndicatorMarketSelectionWidget.
   */
  class OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings :
      public LegacyUI::WindowSettings {
    public:

      //! Constructs an
      //! OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings.
      /*!
        \param widget The widget to represent.
      */
      OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings(
        const OrderImbalanceIndicatorMarketSelectionWidget& widget);

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings::Shuttle(
    Shuttler& shuttle, unsigned int version) {}
}

#endif
