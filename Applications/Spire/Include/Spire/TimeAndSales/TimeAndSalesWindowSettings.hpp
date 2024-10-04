#ifndef SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  /** Stores the window settings for a TimeAndSalesWindow. */
  class TimeAndSalesWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Constructs a TimeAndSalesWindowSettings with default values. */
      TimeAndSalesWindowSettings() = default;

      /**
       * Constructs a TimeAndSalesWindowSettings.
       * @param window The TimeAndSalesWindow to represent.
       * @param user_profile The user's profile.
       */
      TimeAndSalesWindowSettings(
        const TimeAndSalesWindow& window, Beam::Ref<UserProfile> user_profile);

      std::string GetName() const override;

      QWidget* Reopen(Beam::Ref<UserProfile> user_profile) const override;

      void Apply(Beam::Ref<UserProfile> user_profile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_security_view_stack;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeAndSalesWindowSettings::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("security", m_security);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view_stack", m_security_view_stack);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_link_identifier);
    shuttle.Shuttle("geometry", m_geometry);
  }
}

#endif
