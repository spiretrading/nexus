#ifndef SPIRE_STYLE_DROP_DOWN_MENU_ITEM_HPP
#define SPIRE_STYLE_DROP_DOWN_MENU_ITEM_HPP
#include <QWidget>
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a style item in a StyleDropDownMenu.
  class StyleDropDownMenuItem : public QWidget {
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

      //! Returns the style represented by this item.
      TrendLineStyle get_style() const;

      //! Sets the item's highlighted status.
      void set_highlight();

      //! Removes the item's highlighted status.
      void remove_highlight();

      //! Calls the provided slot when the selected signal is triggered.
      /*!
        \param slot The slot to call.
      */
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      TrendLineStyle m_style;
      bool m_is_highlighted;
      int m_line_length;
      int m_padding;
  };
}

#endif
