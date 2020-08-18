#ifndef SPIRE_NUMERIC_INPUT_WIDGET_HPP
#define SPIRE_NUMERIC_INPUT_WIDGET_HPP
#include <QAbstractSpinBox>
#include <QDoubleValidator>
#include <QLocale>

namespace Spire {

  class NumericInputWidget : public QAbstractSpinBox {
    public:

      NumericInputWidget(QWidget* parent = nullptr);

      void set_step(const QString& step);

      void stepBy(int step) override;

    protected:
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      QAbstractSpinBox::StepEnabled stepEnabled() const override;

    private:
      QDoubleValidator* m_validator;
      QLocale m_locale;
      QString m_step;

      void add_step(int step);
      void add_step(int step, Qt::KeyboardModifiers modifiers);
      void set_stylesheet(bool is_up_disabled, bool is_down_disabled);
      QValidator::State validate(QString& text);
      void on_editing_finished();
  };
}

#endif
