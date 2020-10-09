#ifndef SPIRE_UI_VIEWER_WINDOW_HPP
#define SPIRE_UI_VIEWER_WINDOW_HPP
#include <QHash>
#include <QHboxLayout>
#include <QListWidget>
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays a window to test Spire widgets.
  class UiViewerWindow : public Window {
    public:

      //! Constructs a UiViewerWindow.
      /*
        \param parent The parent widget.
      */
      UiViewerWindow(QWidget* parent = nullptr);

    private:
      QHash<QString, QWidget*> m_widgets;
      QListWidget* m_widget_list;
      QHBoxLayout* m_layout;
      ColorSelectorButton* m_color_selector_button;

      void add_widget(const QString& name, QWidget* container_widget);
      void initialize_color_selector_button();
      void on_create_color_button_color(const QString& color,
        QGridLayout* layout);
      void on_set_color_button_color(const QString& color_hex);
  };
}

#endif
