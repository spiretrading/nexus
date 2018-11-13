#ifndef SPIRE_DROPDOWN_MENU_HPP
#define SPIRE_DROPDOWN_MENU_HPP
#include <QComboBox>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a Spire-styled QComboBox.
  class DropdownMenu : public QWidget {
    public:

      //! Constructs a DropdownMenu with specified items. The first item
      //! in the initializer_list is the default selected item.
      /*
        \param items The items to display in the menu.
        \param parent The parent widget to the DropdownMenu.
      */
      DropdownMenu(const std::initializer_list<QString>& items,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QString m_current_text;
      QImage m_dropdown_image;
      DropdownMenuList* m_menu_list;

      void move_menu_list();
      void on_clicked();
  };
}

#endif
