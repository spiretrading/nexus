#ifndef SPIRE_DROP_DOWN_MENU_2_TEST_WIDGET_HPP
#define SPIRE_DROP_DOWN_MENU_2_TEST_WIDGET_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/DropDownMenu2.hpp"

namespace Spire {

  //! Represents a widget for testing a DropDownMenu.
  class DropDownMenu2TestWidget : public QWidget {
    public:

      //! Constructs a DropDownMenu2TestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit DropDownMenu2TestWidget(QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      DropDownMenu2* m_menu;

      void move_menu();
      void toggle_menu_visibility();
  };
}

#endif
