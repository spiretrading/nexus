#ifndef SPIRE_DROPDOWN_MENU_HPP
#define SPIRE_DROPDOWN_MENU_HPP
#include <QComboBox>

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
      void paintEvent(QPaintEvent* event) override;

    private:
      QImage m_dropdown_image;
  };
}

#endif
