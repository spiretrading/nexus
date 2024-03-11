#ifndef TOOLBAR_UI_TESTER_HPP
#define TOOLBAR_UI_TESTER_HPP
#include <QPointer>
#include <QTextEdit>
#include "Spire/Toolbar/SettingsPanel.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

namespace Spire {

  /** Displays a window to test toolbar components. */ 
  class ToolbarUiTester : public QWidget {
    public:

      /**
       * Constructs a ToolbarUiTester.
       * @param parent The parent widget.
       */
      explicit ToolbarUiTester(QWidget* parent = nullptr);

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      TextBox* m_user_name;
      CheckBox* m_mananger_check_box;
      QTextEdit* m_output;
      QPointer<ToolbarWindow> m_toolbar_window;

      void on_toolbar_click();
      void on_open(ToolbarWindow::WindowType window);
      void on_minimize_all();
      void on_restore_all();
      void on_settings(SettingsPanel::Mode mode,
        UserSettings::Categories categories, const std::filesystem::path& path);
      void on_sign_out();
  };
}

#endif
