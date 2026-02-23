#ifndef SPIRE_TIMEANDSALESWINDOWSETTINGS_HPP
#define SPIRE_TIMEANDSALESWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/TickerViewStack.hpp"
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
      friend struct Beam::DataShuttle;
      TimeAndSalesProperties m_properties;
      Nexus::Ticker m_ticker;
      std::string m_name;
      UI::TickerViewStack m_tickerViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      QByteArray m_splitterState;
      QByteArray m_viewHeaderState;
      QByteArray m_snapshotHeaderState;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeAndSalesWindowSettings::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("ticker", m_ticker);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("ticker_view_stack", m_tickerViewStack);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_linkIdentifier);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("splitter_state", m_splitterState);
    shuttle.shuttle("view_header_state", m_viewHeaderState);
    shuttle.shuttle("snapshot_header_state", m_snapshotHeaderState);
  }
}

#endif
