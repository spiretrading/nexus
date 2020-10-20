#ifndef SPIRE_RANGE_INPUT_TEST_WIDGET_HPP
#define SPIRE_RANGE_INPUT_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/RangeInputWidget.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a RangeInputWidget.
  class RangeInputTestWidget : public QWidget {
    public:

      //! Constructs a RangeInputTestWidget.
      /*!
        \param parent The parent widget.
      */
      RangeInputTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      RangeInputWidget* m_range_input;
      QLabel* m_status_label;
      TextInputWidget* m_min_input;
      TextInputWidget* m_max_input;
      TextInputWidget* m_count_input;

      void on_reset_button();
  };
}

#endif
