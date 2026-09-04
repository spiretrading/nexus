#ifndef SPIRE_BOOK_VIEW_WINDOW_SETTINGS_HPP
#define SPIRE_BOOK_VIEW_WINDOW_SETTINGS_HPP
#include <Beam/Serialization/ShuttleUuid.hpp>
#include <boost/uuid/uuid.hpp>
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/TickerView.hpp"

namespace Spire {
  class BookViewWindow;

  /** Stores the window settings for a BookViewWindow. */
  class BookViewWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Constructs a BookViewWindowSettings with default values. */
      BookViewWindowSettings() = default;

      /**
       * Constructs a BookViewWindowSettings.
       * @param window The BookViewWindow to represent.
       */
      explicit BookViewWindowSettings(const BookViewWindow& window);

      std::string GetName() const override;
      QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const override;
      void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      TickerView::State m_ticker_view;
      boost::uuids::uuid m_hub_id;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BookViewWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("ticker_view", m_ticker_view);
    if(version == 0) {
      shuttle.shuttle("identifier", m_identifier);
      shuttle.shuttle("link_identifier", m_link_identifier);
    } else {
      shuttle.shuttle("hub_id", m_hub_id);
    }
    shuttle.shuttle("geometry", m_geometry);
  }
}

#endif
