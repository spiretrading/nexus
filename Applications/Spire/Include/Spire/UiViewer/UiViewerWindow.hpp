#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <unordered_map>
#include <QListWidget>
#include <QSplitter>
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
      std::unordered_map<QString, UiProfile> m_profiles;

      void add(UiProfile profile);
      void on_item_selected(const QListWidgetItem* current,
        const QListWidgetItem* previous);
  };
}

#endif
