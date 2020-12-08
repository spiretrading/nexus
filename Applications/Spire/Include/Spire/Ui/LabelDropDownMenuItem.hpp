#ifndef LABEL_DROP_DOWN_MENU_ITEM_HPP
#define LABEL_DROP_DOWN_MENU_ITEM_HPP
#include "Spire/Ui/DropDownMenuItem2.hpp"

namespace Spire {

  //! Represents a DropDownMenuItem with a text label.
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
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
