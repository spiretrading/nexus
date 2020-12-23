#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <QHash>
#include <QHboxLayout>
#include <QListWidget>
#include "Spire/Ui/Window.hpp"

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
      QListWidget* m_widget_list;

      void on_item_selected(const QListWidgetItem* current,
        const QListWidgetItem* previous);
  };
}

#endif
