#ifndef SPIRE_DROPDOWN_MENU_HPP
#define SPIRE_DROPDOWN_MENU_HPP
#include <QComboBox>

namespace Spire {

  //! Displays a Spire-styled QComboBox.
  class DropdownMenu : public QComboBox {
    public:

      //! Constructs an empty DropdownMenu.
      /*
        \param parent The parent widget to the DropdownMenu.
      */
      DropdownMenu(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QImage m_dropdown_image;
  };
}

#endif
