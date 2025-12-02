#ifndef SPIRE_TOOLBAR_WINDOW_SETTINGS_HPP
#define SPIRE_TOOLBAR_WINDOW_SETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <QPoint>
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

namespace Spire {

  /** Stores the window settings for a Toolbar. */
  class ToolbarWindowSettings : public LegacyUI::WindowSettings {
    public:

      /**
       * Constructs a ToolbarWindowSettings that positions the toolbar at
       * (0, 0).
       */
      ToolbarWindowSettings();

      /**
       * Constructs a ToolbarWindowSettings.
       * @param toolbar The Toolbar to represent.
       */
      explicit ToolbarWindowSettings(const ToolbarWindow& toolbar);

      std::string GetName() const override;
      QWidget* Reopen(Beam::Ref<UserProfile> profile) const override;
      void Apply(Beam::Ref<UserProfile> userProfile, Beam::Out<QWidget> widget)
        const override;

    private:
      friend struct Beam::DataShuttle;
      QPoint m_position;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void ToolbarWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("position", m_position);
  }
}

#endif
