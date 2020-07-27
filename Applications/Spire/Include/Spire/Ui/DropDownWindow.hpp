#ifndef SPIRE_DROP_DOWN_WINDOW_HPP
#define SPIRE_DROP_DOWN_WINDOW_HPP
#include <QWidget>
#include "Spire/Ui/DropShadow.hpp"

namespace Spire {

  //! Represents a pop-up window for displaying a widget.
  class DropDownWindow : public QWidget {
    public:

      //! Constructs a DropDownWindow.
      /*!
        \param is_click_activated True if the DropDownWindow should be shown
                                  or hidden in response to parent widget
                                  mouse/keyboard activation events, false
                                  otherwise.
        \param parent The parent widget. Used to determine the position of the
                      DropDownWindow.
      */
      explicit DropDownWindow(bool is_click_activated,
        QWidget* parent = nullptr);

      virtual ~DropDownWindow() = default;

      //! Sets the widget to display in the DropDownWindow.
      virtual void initialize_widget(QWidget* widget);

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QWidget* m_widget;
      DropShadow* m_shadow;
      bool m_is_click_activated;

      void move_to_parent();
      void swap_visibility();
  };
}

#endif
