#ifndef SPIRE_DATE_INPUT_TEST_WIDGET_HPP
#define SPIRE_DATE_INPUT_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/DateInputWidget.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a DateInputWidget.
  class DateInputTestWidget : public QWidget {
    public:

      //! Constructs a DateInputTestWidget.
      /*!
        \param parent The parent widget.
      */
      DateInputTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      DateInputWidget* m_date_input;
      QLabel* m_status_label;
      TextInputWidget* m_year_input;
      TextInputWidget* m_month_input;
      TextInputWidget* m_day_input;
      FlatButton* m_reset_button;

      void on_reset_button();
  };
}

#endif
