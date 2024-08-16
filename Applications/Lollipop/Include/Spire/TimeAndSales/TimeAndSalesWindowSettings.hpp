#ifndef SPIRE_TIMEANDSALESWINDOWSETTINGS_HPP
#define SPIRE_TIMEANDSALESWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/SecurityViewStack.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class TimeAndSalesWindowSettings
      \brief Stores the window settings for a TimeAndSalesWindow.
   */
  class TimeAndSalesWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a TimeAndSalesWindowSettings with default values.
      TimeAndSalesWindowSettings();

      //! Constructs a TimeAndSalesWindowSettings.
      /*!
        \param window The TimeAndSalesWindow to represent.
        \param userProfile The user's profile.
      */
      TimeAndSalesWindowSettings(const TimeAndSalesWindow& window,
        Beam::Ref<UserProfile> userProfile);

      virtual ~TimeAndSalesWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      TimeAndSalesProperties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      UI::SecurityViewStack m_securityViewStack;
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
