#ifndef SPIRE_DURATION_INPUT_TEST_WIDGET_HPP
#define SPIRE_DURATION_INPUT_TEST_WIDGET_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/DurationInputWidget.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a DurationInputWidget.
  class DurationInputTestWidget : public QWidget {
    public:

      //! Constructs a DurationInputTestWidget.
      /*
        \param parent The parent widget.
      */
      explicit DurationInputTestWidget(QWidget* parent = nullptr);

    private:
      DurationInputWidget* m_duration_input;
      QLabel* m_status_label;
      TextInputWidget* m_hour_input;
      TextInputWidget* m_minute_input;
      TextInputWidget* m_second_input;

      void on_set_button();
      void on_time_committed(const boost::posix_time::time_duration& time);
  };
}

#endif
