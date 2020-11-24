#ifndef SPIRE_TREND_LINE_STYLE_DROP_DOWN_MENU_HPP
#define SPIRE_TREND_LINE_STYLE_DROP_DOWN_MENU_HPP
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Displays a drop down menu with selectable TrendLineStyles.
  class TrendLineStyleDropDownMenu : public StaticDropDownMenu {
    public:

      //! Signals that a trend line style was selected.
      /*!
        \param style The trend line style that was selected.
      */
      using StyleSignal = Signal<void (TrendLineStyle style)>;

      //! Constructs a TrendLineStyleDropDownMenu.
      /*!
        \param parent The parent widget to the TrendLineStyleDropDownMenu.
      */
      explicit TrendLineStyleDropDownMenu(QWidget* parent = nullptr);

      //! Returns the selected item.
      TrendLineStyle get_style() const;

      //! Calls the provided slot when the style signal is triggered.
      /*!
        \param slot The slot to call.
      */
      boost::signals2::connection connect_style_signal(
        const StyleSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable StyleSignal m_style_signal;
      boost::signals2::scoped_connection m_selected_connection;
      TrendLineStyle m_current_style;
      QImage m_dropdown_image;

      void on_item_selected(TrendLineStyle style);
  };
}

#endif
