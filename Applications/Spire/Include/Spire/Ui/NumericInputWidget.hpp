#ifndef SPIRE_NUMERIC_INPUT_WIDGET_HPP
#define SPIRE_NUMERIC_INPUT_WIDGET_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <QAbstractSpinBox>
#include <QLocale>
#include <QRegularExpression>

namespace Spire {

  class NumericInputWidget : public QAbstractSpinBox {
    public:

      static const auto MAX_DECIMAL_PLACES = 15;

      using Real = boost::multiprecision::cpp_dec_float<MAX_DECIMAL_PLACES>;

      //! Constructs a NumericInputWidget with a default step of 1.
      /*!
        \param parent The parent widget.
      */
      NumericInputWidget(Real value, QWidget* parent = nullptr);

      //! Sets the number of decimal places that can be input, up to
      //! MAX_DECIMAL_PLACES.
      /*!
        \param decimals The number of accepted decimal places.
      */
      void set_decimals(int decimals);

      //! Sets the number minimum number of displayed decimals to display
      //! when a value is committed.
      /*!
        \param decimals The minimum number of decimals to display.
      */
      void set_minimum_decimals(int decimals);

      void set_minimum(Real minimum);

      void set_maximum(Real maximum);

      void set_step(Real step);

      void stepBy(int step) override;

    protected:
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      QAbstractSpinBox::StepEnabled stepEnabled() const override;

    private:
      QRegularExpression m_real_regex;
      QLocale m_locale;
      Real m_minimum;
      Real m_maximum;
      int m_decimals;
      int m_minimum_decimals;
      Real m_step;

      void add_step(int step);
      void add_step(int step, Qt::KeyboardModifiers modifiers);
      QString display_string(Real value);
      bool is_valid(const QString& string);
      void set_stylesheet(bool is_up_disabled, bool is_down_disabled);
      void update_stylesheet();
      void on_editing_finished();
  };
}

#endif
