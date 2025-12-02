#ifndef SPIRE_BOOKVIEWWINDOWSETTINGS_HPP
#define SPIRE_BOOKVIEWWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
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
      friend struct Beam::DataShuttle;
      BookViewProperties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_securityViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      QByteArray m_bidPanelHeader;
      QByteArray m_askPanelHeader;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BookViewWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("security", m_security);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("security_view_stack", m_securityViewStack);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_linkIdentifier);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("bid_panel_header", m_bidPanelHeader);
    shuttle.shuttle("ask_panel_header", m_askPanelHeader);
  }
}

#endif
