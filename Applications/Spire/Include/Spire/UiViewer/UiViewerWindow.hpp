#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <any>
#include <unordered_map>
#include <vector>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include "Spire/Ui/Window.hpp"
#include "Spire/UiViewer/UiProfile.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Displays a window to test Spire widgets.
  class UiViewerWindow : public Window {
    public:

      //! Constructs a UiViewerWindow.
      /*
        \param parent The parent widget.
      */
      explicit UiViewerWindow(QWidget* parent = nullptr);

    private:
      QSplitter* m_body;
      QListWidget* m_widget_list;
      int m_line_count;
      QSplitter* m_stage;
      QScrollArea* m_center_stage;
      QTextEdit* m_event_log;
      QPushButton* m_reset_button;
      QPushButton* m_rebuild_button;
      std::unordered_map<QString, UiProfile> m_profiles;

      void add(UiProfile profile);
      void update_table(const UiProfile& profile);
      void on_event(const QString& name,
        const std::vector<std::any>& arguments);
      void on_item_selected(const QListWidgetItem* current,
        const QListWidgetItem* previous);
      void on_reset();
      void on_rebuild();
  };
}

#endif
