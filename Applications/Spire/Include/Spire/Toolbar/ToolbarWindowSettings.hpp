#ifndef SPIRE_TOOLBAR_WINDOW_SETTINGS_HPP
#define SPIRE_TOOLBAR_WINDOW_SETTINGS_HPP
#include <Beam/Serialization/Serialization.hpp>
#include <QPoint>
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/LegacyUI/ShuttleQtTypes.hpp"
#include "Spire/Toolbar/Toolbar.hpp"

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
      friend struct Beam::Serialization::DataShuttle;
      QPoint m_position;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ToolbarWindowSettings::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("position", m_position);
  }
}

#endif
