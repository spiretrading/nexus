#ifndef SPIRE_TIME_INPUT_WIDGET_HPP
#define SPIRE_TIME_INPUT_WIDGET_HPP
#include <QTimeEdit>
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

    private:
      mutable TimeChangeSignal m_time_signal;
      QTimeEdit* m_time_edit;
      DropDownMenu* m_drop_down_menu;

      void on_drop_down_changed(const QString& string);
      void on_time_changed(const QTime& time);
  };
}

#endif
