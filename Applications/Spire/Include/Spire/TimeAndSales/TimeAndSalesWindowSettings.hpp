#ifndef SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/ShuttleQtTypes.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {

  /** Stores the window settings for a TimeAndSalesWindow. */
  class TimeAndSalesWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Constructs a TimeAndSalesWindowSettings with default values. */
      TimeAndSalesWindowSettings() = default;

      /**
       * Constructs a TimeAndSalesWindowSettings.
       * @param window The TimeAndSalesWindow to represent.
       * @param userProfile The user's profile.
       */
      TimeAndSalesWindowSettings(const TimeAndSalesWindow& window,
        Beam::Ref<UserProfile> userProfile);

      std::string GetName() const override;

      QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const override;

      void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::Serialization::DataShuttle;
      TimeAndSalesProperties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_securityViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      QByteArray m_splitterState;
      QByteArray m_viewHeaderState;
      QByteArray m_snapshotHeaderState;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeAndSalesWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("security", m_security);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view_stack", m_securityViewStack);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_linkIdentifier);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("splitter_state", m_splitterState);
    shuttle.Shuttle("view_header_state", m_viewHeaderState);
    shuttle.Shuttle("snapshot_header_state", m_snapshotHeaderState);
  }
}

#endif
