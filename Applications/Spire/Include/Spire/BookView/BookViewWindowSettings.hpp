#ifndef SPIRE_BOOK_VIEW_WINDOW_SETTINGS_HPP
#define SPIRE_BOOK_VIEW_WINDOW_SETTINGS_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/SecurityView.hpp"

namespace Spire {

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

      QWidget* Reopen(Beam::Ref<UserProfile> user_profile) const override;

      void Apply(Beam::Ref<UserProfile> user_profile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_name;
      SecurityView::State m_security_view;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BookViewWindowSettings::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view", m_security_view);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_link_identifier);
    shuttle.Shuttle("geometry", m_geometry);
  }
}

#endif
