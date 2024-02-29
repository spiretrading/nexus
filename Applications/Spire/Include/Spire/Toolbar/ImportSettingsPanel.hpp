#ifndef SPIRE_IMPORT_SETTINGS_PANEL_HPP
#define SPIRE_IMPORT_SETTINGS_PANEL_HPP
#include <QWidget>
#include <Beam/Collections/EnumSet.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a panel to import settings from a file. */
  class ImportSettingsPanel : public QWidget {
    public:

      /**
       * Signals to import the selected settings.
       * @param settings The set of settings to import.
       */
      using ImportSignal = Signal<void (Settings settings)>;

      /**
       * Constructs an ImportSettingsPanel initialized with an empty set of
       * settings.
       * @param parent The parent widget.
       */
      explicit ImportSettingsPanel(QWidget& parent);

      /**
       * Constructs an ImportSettingsPanel.
       * @param settings The settings to import.
       * @param parent The parent widget.
       */
      ImportSettingsPanel(
        std::shared_ptr<SettingsModel> settings, QWidget& parent);

      /** Returns the settings. */
      const std::shared_ptr<SettingsModel>& get_settings() const;

      /** Connects a slot to the ImportSignal. */
      boost::signals2::connection connect_import_signal(
        const ImportSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable ImportSignal m_import_signal;
      std::shared_ptr<SettingsModel> m_settings;
      Button* m_import_button;
      OverlayPanel* m_panel;
      boost::signals2::scoped_connection m_connection;

      void on_update(const Settings& settings);
      void on_cancel();
      void on_import();
  };

  /** Returns the text representation of a Setting. */ 
  const QString& to_text(Setting setting);
}

#endif
