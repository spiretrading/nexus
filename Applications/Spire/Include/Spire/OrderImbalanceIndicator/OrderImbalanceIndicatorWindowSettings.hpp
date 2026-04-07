#ifndef SPIRE_ORDERIMBALANCEINDICATORWINDOWSETTINGS_HPP
#define SPIRE_ORDERIMBALANCEINDICATORWINDOWSETTINGS_HPP
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <QByteArray>
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class OrderImbalanceIndicatorWindowSettings
      \brief Stores the window settings for an OrderImbalanceIndicatorWindow.
   */
  class OrderImbalanceIndicatorWindowSettings :
      public LegacyUI::WindowSettings {
    public:

      //! Constructs an OrderImbalanceIndicatorWindowSettings with default
      //! values.
      OrderImbalanceIndicatorWindowSettings();

      //! Constructs an OrderImbalanceIndicatorWindowSettings.
      /*!
        \param window The OrderImbalanceIndicatorWindow to represent.
        \param userProfile The user's profile.
      */
      OrderImbalanceIndicatorWindowSettings(
        const OrderImbalanceIndicatorWindow& window,
        Beam::Ref<UserProfile> userProfile);

      virtual ~OrderImbalanceIndicatorWindowSettings();

      virtual std::string GetName() const;
      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;
      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      OrderImbalanceIndicatorProperties m_properties;
      QByteArray m_geometry;
      QByteArray m_tableHeaderGeometry;
      QByteArray m_tableHeaderState;
      std::shared_ptr<LegacyUI::WindowSettings> m_timeRangeSettings;
      std::shared_ptr<LegacyUI::WindowSettings> m_marketsSettings;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderImbalanceIndicatorWindowSettings::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("table_header_geometry", m_tableHeaderGeometry);
    shuttle.shuttle("table_header_state", m_tableHeaderState);
    shuttle.shuttle("time_range_settings", m_timeRangeSettings);
    shuttle.shuttle("markets_settings", m_marketsSettings);
  }
}

#endif
