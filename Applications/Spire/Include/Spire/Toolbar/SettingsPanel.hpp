#ifndef SPIRE_SETTINGS_PANEL_HPP
#define SPIRE_SETTINGS_PANEL_HPP
#include <QWidget>
#include "Spire/Toolbar/UserSettings.hpp"

namespace Spire {
  class Button;
  class OverlayPanel;

  /** Displays a panel for selecting settings to import or export. */
  class SettingsPanel : public QWidget {
    public:

      /** Specifies whether the settings are being imported or exported. */
      enum class Mode {

        /** The settings panel is importing settings. */
        IMPORT,

        /** The settings panel is exporting settings. */
        EXPORT
      };

      /**
       * Signals to commit the settings.
       * @param categories The setting categories to import.
       */
      using CommitSignal = Signal<void (UserSettings::Categories categories)>;

      /**
       * Constructs a SettingsPanel initialized with an empty set of settings.
       * @param mode Whether to import or export settings.
       * @param parent The parent widget.
       */
      SettingsPanel(Mode mode, QWidget& parent);

      /**
       * Constructs a SettingsPanel.
       * @param mode Whether to import or export settings.
       * @param categories The selected settings.
       * @param parent The parent widget.
       */
      SettingsPanel(
        Mode mode, std::shared_ptr<UserSettings::CategoriesModel> categories,
        QWidget& parent);

      /** Returns the selected settings. */
      const std::shared_ptr<UserSettings::CategoriesModel>&
        get_categories() const;

      /** Connects a slot to the CommitSignal. */
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable CommitSignal m_commit_signal;
      std::shared_ptr<UserSettings::CategoriesModel> m_categories;
      Button* m_commit_button;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_connection;

      void on_update(const UserSettings::Categories& categories);
      void on_cancel();
      void on_commit();
  };
}

#endif
