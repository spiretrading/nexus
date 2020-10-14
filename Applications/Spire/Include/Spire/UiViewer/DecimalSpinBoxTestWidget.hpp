#ifndef SPIRE_DECIMAL_SPIN_BOX_TEST_WIDGET_HPP
#define SPIRE_DECIMAL_SPIN_BOX_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include "Spire/Ui/DecimalSpinBox.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a DecimalSpinBox.
  class DecimalSpinBoxTestWidget : public QWidget {
    public:

      //! Constructs a DecimalSpinBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      DecimalSpinBoxTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      std::shared_ptr<DecimalSpinBoxModel> m_model;
      DecimalSpinBox* m_spin_box;
      QLabel* m_value_label;
      TextInputWidget* m_initial_input;
      TextInputWidget* m_min_input;
      TextInputWidget* m_max_input;
      TextInputWidget* m_increment_input;
      StaticDropDownMenu* m_modifier_menu;

      void reset_spin_box();
      void on_initial_set();
      void on_min_set();
      void on_max_set();
      void on_increment_set();
  };
}

#endif
