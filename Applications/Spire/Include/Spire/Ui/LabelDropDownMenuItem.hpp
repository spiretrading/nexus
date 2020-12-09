#ifndef LABEL_DROP_DOWN_MENU_ITEM_HPP
#define LABEL_DROP_DOWN_MENU_ITEM_HPP
#include "Spire/Ui/DropDownMenuItem2.hpp"

namespace Spire {

  //! Implements a DropDownMenuItem displaying the text representation of its
  //! value.
  class LabelDropDownMenuItem : public DropDownMenuItem2 {
    public:

      //! Constructs a LabelDropDownMenuItem.
      /*!
        \param value The item's value.
        \param parent The parent widget.
      */
      explicit LabelDropDownMenuItem(QVariant value,
        QWidget* parent = nullptr);

    protected:
      void enterEvent(QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
