#ifndef SPIRE_TIME_INPUT_WIDGET_HPP
#define SPIRE_TIME_INPUT_WIDGET_HPP
#include <QWidget>
#include "Spire/Spire/Scalar.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a time input widget.
  class TimeInputWidget : public QWidget {
    public:

      //! Signals that the time has been modified.
      using TimeChangeSignal = Signal<void (Scalar)>;

      //! Constructs a TimeInputWidget.
      TimeInputWidget(QWidget* parent = nullptr);

      //! Sets the widget's time without triggering the time change signal.
      void set_time(Scalar time);

      //! Connects a slot to the time signal.
      boost::signals2::connection connect_time_signal(
        const TimeChangeSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable TimeChangeSignal m_time_signal;
      QLineEdit* m_hour_line_edit;
      QLineEdit* m_minute_line_edit;
      DropDownMenu* m_drop_down_menu;

      void set_focus_style();
      void set_unfocused_style();
      void set_style(const QString& border_hex);
      void on_drop_down_changed(const QString& item);
      void on_time_changed();
  };
}

#endif
