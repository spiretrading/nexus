#ifndef SPIRE_TIME_OF_DAY_INPUT_WIDGET_HPP
#define SPIRE_TIME_OF_DAY_INPUT_WIDGET_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QPainter>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a time of day input widget.
  class TimeOfDayInputWidget : public QWidget {
    public:

      //! Signals that the time of day has been modified.
      using TimeChangeSignal = Signal<void (
        const boost::posix_time::time_duration&)>;

      //! Constructs a TimeOfDayInputWidget.
      TimeOfDayInputWidget(QWidget* parent = nullptr);

      //! Sets the widget's time.
      void set_time(const boost::posix_time::time_duration& time);

      //! Connects a slot to the time signal.
      boost::signals2::connection connect_time_signal(
        const TimeChangeSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable TimeChangeSignal m_time_signal;
      QLineEdit* m_hour_line_edit;
      ColonWidget* m_colon_widget;
      QLineEdit* m_minute_line_edit;
      StaticDropDownMenu* m_drop_down_menu;
      int m_last_valid_hour;
      int m_last_valid_minute;

      QString clamped_value(const QString& text, int min_value, int max_value);
      QString clamped_value(const QString& text, int min_value, int max_value,
        int addend);
      QString get_line_edit_value(const QString& text, int key, int min_value,
        int max_value);
      void set_focus_style();
      void set_unfocused_style();
      void set_style(const QString& border_hex);
      void on_drop_down_changed(const QString& item);
      void on_time_changed();
  };
}

#endif
