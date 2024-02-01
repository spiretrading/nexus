#ifndef SPIRE_BOOKVIEWWINDOWSETTINGS_HPP
#define SPIRE_BOOKVIEWWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/ShuttleQtTypes.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class BookViewWindowSettings
      \brief Stores the window settings for a BookViewWindow.
   */
  class BookViewWindowSettings : public LegacyUI::WindowSettings {
    public:

      //! Constructs a BookViewWindowSettings with default values.
      BookViewWindowSettings();

      //! Constructs a BookViewWindowSettings.
      /*!
        \param window The BookViewWindow to represent.
        \param userProfile The user's profile.
      */
      BookViewWindowSettings(const BookViewWindow& window,
        Beam::Ref<UserProfile> userProfile);

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      BookViewProperties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_securityViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      QByteArray m_bidPanelHeader;
      QByteArray m_askPanelHeader;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BookViewWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("security", m_security);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view_stack", m_securityViewStack);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_linkIdentifier);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("bid_panel_header", m_bidPanelHeader);
    shuttle.Shuttle("ask_panel_header", m_askPanelHeader);
  }
}

#endif
