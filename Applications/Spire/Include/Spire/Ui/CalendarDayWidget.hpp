#ifndef SPIRE_CALENDAR_DAY_WIDGET_HPP
#define SPIRE_CALENDAR_DAY_WIDGET_HPP
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <QLabel>
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a date on a calendar.
  class CalendarDayWidget : public QLabel {
    public:

      //! Signals that this day widget was selected.
      using ClickedSignal = Signal<void (boost::gregorian::date)>;

      //! Constructs a CalendarDayWidget with a date to display and a custom
      //! text color.
      /*
        \param displayed_date The date this widget represents.
        \param text_color The text color.
        \param parent The parent widget.
      */
      CalendarDayWidget(boost::gregorian::date displayed_date,
        const QColor& text_color, QWidget* parent = nullptr);

      //! Sets the highlighted style.
      void set_highlight();

      //! Sets the default style.
      void remove_highlight();

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      boost::gregorian::date m_date;
      QColor m_text_color;

      void set_default_style();
      void set_selected_style();
  };
}

#endif
