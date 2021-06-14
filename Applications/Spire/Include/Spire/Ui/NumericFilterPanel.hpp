#ifndef SPIRE_NUMERIC_FILTER_PANEL_HPP
#define SPIRE_NUMERIC_FILTER_PANEL_HPP
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a panel for filtering numeric values.
   */
  template<typename T>
  class NumericFilterPanel : public QWidget {
    public:

      /** The component type of inputting numeric value. */
      using NumericBox = T;

      /** The type of the numeric value. */
      using Type = typename
        std::decay_t<decltype(*std::declval<NumericBox>().get_model())>::Scalar;

      /** A ScalarValueModel over optional Type. */
      using OptionalNumericModel = ScalarValueModel<boost::optional<Type>>;

      /** A LocalScalarValueModel over optional Type. */
      using LocalOptionalNumericModel =
        LocalScalarValueModel<boost::optional<Type>>;

      /**
       * Signals that the minimum value and the maximum value for the filter was
       * submitted.
       * @param min_value The minimum value.
       * @param max_value The maximum value.
       */
      using FilterSignal = Signal<void (const boost::optional<Type>& min_value,
        const boost::optional<Type>& max_value)>;

      /**
       * Constructs a NumericFilterPanel with a LocalOptionalNumericModel for
       * the minimum value and a LocalOptionalNumericModel for the maximum value.
       * @param default_min_value The default minimum value.
       * @param default_max_value The default maximum value.
       * @param title The title of the panel.
       * @param parent The parent widget that shows the panel.
       */
      NumericFilterPanel(boost::optional<Type> default_min_value,
        boost::optional<Type> default_max_value, QString title,
        QWidget* parent = nullptr);

      /**
       * Constructs a NumericFilterPanel.
       * @param min_model The OptionalNumericModel used for the minimum value.
       * @param max_model The OptionalNumericModel used for the maximum value.
       * @param default_min_value The default minimum value.
       * @param default_max_value The default maximum value.
       * @param title The title of the panel.
       * @param parent The parent widget that shows the panel.
       */
      NumericFilterPanel(std::shared_ptr<OptionalNumericModel> min_model,
        std::shared_ptr<OptionalNumericModel> max_model,
        boost::optional<Type> default_min_value,
        boost::optional<Type> default_max_value,
        QString title, QWidget* parent = nullptr);

      /** Returns the model of the minimum value. */
      const std::shared_ptr<OptionalNumericModel>& get_min_model() const;

      /** Returns the model of the maximum value. */
      const std::shared_ptr<OptionalNumericModel>& get_max_model() const;

      /** Returns the default minimum value. */
      boost::optional<Type> get_default_min_value() const;

      /** Returns the default maximum value. */
      boost::optional<Type> get_default_max_value() const;

      /** Sets the default minimum and maximum value. */
      void set_default_value(const boost::optional<Type>& default_min,
        const boost::optional<Type>& default_max);

      /** Connects a slot to the filter signal. */
      boost::signals2::connection connect_filter_signal(
        typename const FilterSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      mutable FilterSignal m_filter_signal;
      std::shared_ptr<OptionalNumericModel> m_min_model;
      std::shared_ptr<OptionalNumericModel> m_max_model;
      boost::optional<Type> m_default_min_value;
      boost::optional<Type> m_default_max_value;
      boost::optional<Type> m_min_value;
      boost::optional<Type> m_max_value;
      FilterPanel* m_filter_panel;
      NumericBox* m_min_input;
      NumericBox* m_max_input;

      static NumericBox* make_input_field(
        std::shared_ptr<OptionalNumericModel> model);
      static QHBoxLayout* make_row_layout(QString label_name,
        NumericBox* input_field);
      void on_min_input_submit(const boost::optional<Type>& value);
      void on_max_input_submit(const boost::optional<Type>& value);
      void reset();
  };

  template<typename T>
  NumericFilterPanel<T>::NumericFilterPanel(
    boost::optional<Type> default_min_value,
    boost::optional<Type> default_max_value,
    QString tilte, QWidget* parent)
    : NumericFilterPanel(std::make_shared<LocalOptionalNumericModel>(),
      std::make_shared<LocalOptionalNumericModel>(), default_min_value,
      default_max_value, title, parent) {}

  template<typename T>
  NumericFilterPanel<T>::NumericFilterPanel(
      std::shared_ptr<OptionalNumericModel> min_model,
      std::shared_ptr<OptionalNumericModel> max_model,
      boost::optional<Type> default_min_value,
      boost::optional<Type> default_max_value,
      QString title, QWidget* parent)
      : QWidget(parent),
        m_min_model(std::move(min_model)),
        m_max_model(std::move(max_model)),
        m_default_min_value(std::move(default_min_value)),
        m_default_max_value(std::move(default_max_value)),
        m_min_value(m_min_model->get_current()),
        m_max_value(m_max_model->get_current()) {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    m_min_input = make_input_field(m_min_model);
    auto min_layout = make_row_layout(tr("Min"), m_min_input);
    layout->addLayout(min_layout);
    layout->addSpacing(scale_height(10));
    m_max_input = make_input_field(m_max_model);
    auto max_layout = make_row_layout(tr("Max"), m_max_input);
    layout->addLayout(max_layout);
    m_filter_panel = new FilterPanel(std::move(title), this, parent);
    m_filter_panel->connect_reset_signal([=] {
      reset();
    });
    m_min_input->connect_submit_signal([=] (const auto& value) {
      on_min_input_submit(value);
    });
    m_max_input->connect_submit_signal([=] (const auto& value) {
      on_max_input_submit(value);
    });
  }

  template<typename T>
  const std::shared_ptr<typename NumericFilterPanel<T>::OptionalNumericModel>&
      NumericFilterPanel<T>::get_min_model() const {
    return m_min_model;
  }

  template<typename T>
  const std::shared_ptr<typename NumericFilterPanel<T>::OptionalNumericModel>&
      NumericFilterPanel<T>::get_max_model() const {
    return m_max_model;
  }

  template<typename T>
  boost::optional<typename NumericFilterPanel<T>::Type>
      NumericFilterPanel<T>::get_default_min_value() const {
    return m_default_min_value;
  }

  template<typename T>
  boost::optional<typename NumericFilterPanel<T>::Type>
      NumericFilterPanel<T>::get_default_max_value() const {
    return m_default_max_value;
  }

  template<typename T>
  void NumericFilterPanel<T>::set_default_value(
      const boost::optional<Type>& default_min,
      const boost::optional<Type>& default_max) {
    m_default_min = default_min;
    m_default_max = default_max;
  }

  template<typename T>
  boost::signals2::connection NumericFilterPanel<T>::connect_filter_signal(
      typename const FilterSignal::slot_type& slot) const {
    return m_filter_signal.connect(slot);
  }

  template<typename T>
  bool NumericFilterPanel<T>::event(QEvent* event) {
    if(event->type() == QEvent::ShowToParent) {
      m_filter_panel->show();
      m_min_input->setFocus(Qt::TabFocusReason);
    }
    return QWidget::event(event);
  }

  template<typename T>
  typename NumericFilterPanel<T>::NumericBox*
      NumericFilterPanel<T>::make_input_field(
        std::shared_ptr<OptionalNumericModel> model) {
    auto field = [&] {
      if constexpr(std::is_same_v<NumericBox, DurationBox>) {
        return new NumericBox(std::move(model));
      }
      auto modifiers = QHash<Qt::KeyboardModifier, Type>(
        {{Qt::NoModifier, model->get_increment()},
        {Qt::AltModifier, 5 * model->get_increment()},
        {Qt::ControlModifier, 10 * model->get_increment()},
        {Qt::ShiftModifier, 20 * model->get_increment()}});
      return new NumericBox(std::move(model), std::move(modifiers));
    }();
    field->setFixedSize(scale(120, 26));
    return field;
  }

  template<typename T>
  QHBoxLayout* NumericFilterPanel<T>::make_row_layout(QString label_name,
      NumericBox* input_field) {
    auto layout = new QHBoxLayout();
    auto label = new TextBox(std::move(label_name));
    label->set_read_only(true);
    label->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(label);
    layout->addSpacing(scale_width(18));
    layout->addStretch();
    layout->addWidget(input_field);
    return layout;
  }

  template<typename T>
  void NumericFilterPanel<T>::on_min_input_submit(
      const boost::optional<Type>& value) {
    if(m_max_value && value && *value > *m_max_value) {
      m_max_value = value;
      m_max_model->set_current(m_max_value);
    }
    if(m_min_value != value) {
      m_filter_signal(value, m_max_value);
    }
    m_min_value = value;
  }

  template<typename T>
  void NumericFilterPanel<T>::on_max_input_submit(
      const boost::optional<Type>& value) {
    if(m_min_value && value && *value < *m_min_value) {
      m_min_value = value;
      m_min_model->set_current(m_min_value);
    }
    if(m_max_value != value) {
      m_filter_signal(m_min_value, value);
    }
    m_max_value = value;
  }

  template<typename T>
  void NumericFilterPanel<T>::reset() {
    m_min_model->set_current(m_default_min_value);
    m_max_model->set_current(m_default_max_value);
    if(m_min_value != m_default_min_value ||
        m_max_value != m_default_max_value) {
      m_filter_signal(m_default_min_value, m_default_max_value);
      m_min_value = m_default_min_value;
      m_max_value = m_default_max_value;
    }
  }

  using DecimalFilterPanel = NumericFilterPanel<DecimalBox>;
  using DurationFilterPanel = NumericFilterPanel<DurationBox>;
  using IntegerFilterPanel = NumericFilterPanel<IntegerBox>;
  using MoneyFilterPanel = NumericFilterPanel<MoneyBox>;
}

#endif
