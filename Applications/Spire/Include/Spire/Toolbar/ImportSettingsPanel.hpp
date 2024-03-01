#ifndef SPIRE_IMPORT_SETTINGS_PANEL_HPP
#define SPIRE_IMPORT_SETTINGS_PANEL_HPP
#include <QWidget>
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Toolbar/UserSettings.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a panel to import settings from a file. */
  class ImportSettingsPanel : public QWidget {
    public:

      /**
       * Signals to import the selected settings.
       * @param categories The setting categories to import.
       */
      using ImportSignal = Signal<void (UserSettings::Categories categories)>;

      /**
       * Constructs an ImportSettingsPanel initialized with an empty set of
       * settings.
       * @param parent The parent widget.
       */
      explicit ImportSettingsPanel(QWidget& parent);

      /**
       * Constructs an ImportSettingsPanel.
       * @param categories The settings to import.
       * @param parent The parent widget.
       */
      ImportSettingsPanel(
        std::shared_ptr<UserSettings::CategoriesModel> categories,
        QWidget& parent);

      /** Returns the categories to import. */
      const std::shared_ptr<UserSettings::CategoriesModel>&
        get_categories() const;

      /** Connects a slot to the ImportSignal. */
      boost::signals2::connection connect_import_signal(
        const ImportSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable ImportSignal m_import_signal;
      std::shared_ptr<UserSettings::CategoriesModel> m_categories;
      Button* m_import_button;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_connection;

      void on_update(const UserSettings::Categories& categories);
      void on_cancel();
      void on_import();
  };
}

#endif
