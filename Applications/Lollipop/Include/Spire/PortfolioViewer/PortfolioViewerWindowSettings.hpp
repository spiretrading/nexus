#ifndef SPIRE_PORTFOLIOVIEWERWINDOWSETTINGS_HPP
#define SPIRE_PORTFOLIOVIEWERWINDOWSETTINGS_HPP
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class PortfolioViewerWindowSettings
      \brief Stores the window settings for a PortfolioViewerWindow.
   */
  class PortfolioViewerWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a PortfolioViewerWindowSettings with default values.
      PortfolioViewerWindowSettings();

      //! Constructs a PortfolioViewerWindowSettings.
      /*!
        \param window The PortfolioViewerWindow to represent.
      */
      PortfolioViewerWindowSettings(const PortfolioViewerWindow& window);

      virtual ~PortfolioViewerWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      PortfolioViewerProperties m_properties;
      QByteArray m_geometry;
      QByteArray m_portfolioTableHeaderState;
      QByteArray m_splitterState;
      QByteArray m_selectionTableHeaderState;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void PortfolioViewerWindowSettings::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("portfolio_table_header_state",
      m_portfolioTableHeaderState);
    shuttle.shuttle("splitter_state", m_splitterState);
    shuttle.shuttle("selection_table_header_state",
      m_selectionTableHeaderState);
  }
}

#endif
