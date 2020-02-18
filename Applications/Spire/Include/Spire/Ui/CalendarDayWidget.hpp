#ifndef SPIRE_CALENDAR_DAY_WIDGET_HPP
#define SPIRE_CALENDAR_DAY_WIDGET_HPP
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <QLabel>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class CalendarDayWidget : public QLabel {
    public:

      using ClickedSignal = Signal<void (boost::gregorian::date)>;

      CalendarDayWidget(boost::gregorian::date displayed_date,
        const QString& text_color_hex, QWidget* parent = nullptr);

      void set_highlight();

      void remove_highlight();

      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      boost::gregorian::date m_date;
      QString m_text_color_hex;

      void set_default_style();
      void set_selected_style();
  };
}

#endif
