#ifndef SPIRE_PORTFOLIOVIEWERWINDOWSETTINGS_HPP
#define SPIRE_PORTFOLIOVIEWERWINDOWSETTINGS_HPP
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class PortfolioViewerWindowSettings
      \brief Stores the window settings for a PortfolioViewerWindow.
   */
  class PortfolioViewerWindowSettings : public LegacyUI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;
      std::string m_name;
      PortfolioViewerProperties m_properties;
      QByteArray m_geometry;
      QByteArray m_portfolioTableHeaderState;
      QByteArray m_splitterState;
      QByteArray m_selectionTableHeaderState;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void PortfolioViewerWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("portfolio_table_header_state",
      m_portfolioTableHeaderState);
    shuttle.Shuttle("splitter_state", m_splitterState);
    shuttle.Shuttle("selection_table_header_state",
      m_selectionTableHeaderState);
  }
}

#endif
