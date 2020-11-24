#ifndef SPIRE_STYLE_DROP_DOWN_MENU_ITEM_HPP
#define SPIRE_STYLE_DROP_DOWN_MENU_ITEM_HPP
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Ui/DropDownItem.hpp"

namespace Spire {

  //! Represents a style item in a StyleDropDownMenu.
  class StyleDropDownMenuItem : public DropDownItem {
    public:

      //! Signals that a user selected a trend line style.
      /*!
        \param style The selected trend line style.
      */
      using SelectedSignal = Signal<void (TrendLineStyle style)>;

      //! Constructs a DropDownMenuItem with a specified style.
      /*!
        \param style The style to display.
        \param parent The parent to this widget.
      */
      explicit StyleDropDownMenuItem(TrendLineStyle style,
        QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      TrendLineStyle m_style;
  };
}

#endif
