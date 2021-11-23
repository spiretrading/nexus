#ifndef SPIRE_SCALAR_FILTER_PANEL_HPP
#define SPIRE_SCALAR_FILTER_PANEL_HPP
#include <type_traits>
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
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a FilterPanel over a range of scalar values.
   * @param <T> The type of component used to input the scalar values.
   */
  template<typename T>
  class ScalarFilterPanel : public QWidget {
    public:

      /** The type of component used to input the scalar values. */
      using ScalarBox = T;

      /** The type of scalar to filter. */
      using Type = typename std::remove_reference_t<
        decltype(*std::declval<ScalarBox>().get())>::Scalar;

      /** The type of model representing the min and max range values. */
      using Model = ScalarValueModel<boost::optional<Type>>;

      /**
       * Signals that the minimum and maximum value for the filter was
       * submitted.
       * @param min The minimum value.
       * @param max The maximum value.
       */
      using SubmitSignal = Signal<void (
        const boost::optional<Type>& min, const boost::optional<Type>& max)>;

      /**
       * Constructs a ScalarFilterPanel using a local Model for the minimum and
       * maximum values.
       * @param default_min The default minimum value.
       * @param default_max The default maximum value.
       * @param title The title of the panel.
       * @param parent The parent widget showing the panel.
       */
      ScalarFilterPanel(boost::optional<Type> default_min,
        boost::optional<Type> default_max, QString title, QWidget& parent);

      /**
       * Constructs a ScalarFilterPanel.
       * @param min The Model used for the minimum value.
       * @param max The Model used for the maximum value.
       * @param default_min The default minimum value.
       * @param default_max The default maximum value.
       * @param title The title of the panel.
       * @param parent The parent widget that shows the panel.
       */
      ScalarFilterPanel(std::shared_ptr<Model> min, std::shared_ptr<Model> max,
        boost::optional<Type> default_min, boost::optional<Type> default_max,
        QString title, QWidget& parent);

      /** Returns the minimum value model. */
      const std::shared_ptr<Model>& get_min() const;

      /** Returns the maximum value model. */
      const std::shared_ptr<Model>& get_max() const;

      /** Returns the default minimum value. */
      const boost::optional<Type>& get_default_min() const;

      /** Returns the default maximum value. */
      const boost::optional<Type>& get_default_max() const;

      /** Sets the default minimum and maximum values. */
      void set_defaults(const boost::optional<Type>& default_min,
        const boost::optional<Type>& default_max);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<Model> m_min;
      std::shared_ptr<Model> m_max;
      boost::optional<Type> m_default_min;
      boost::optional<Type> m_default_max;
      FilterPanel* m_filter_panel;
      ScalarBox* m_min_box;
      ScalarBox* m_max_box;

      static ScalarBox* make_scalar_box(const Model& model);
      static QHBoxLayout* make_row_layout(QString label, ScalarBox& box);
      void on_reset();
      void on_submit_min(const boost::optional<Type>& submission);
      void on_submit_max(const boost::optional<Type>& submission);
  };

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(boost::optional<Type> default_min,
    boost::optional<Type> default_max, QString title, QWidget& parent)
    : ScalarFilterPanel(
        std::make_shared<LocalScalarValueModel<boost::optional<Type>>>(),
        std::make_shared<LocalScalarValueModel<boost::optional<Type>>>(),
        std::move(default_min), std::move(default_max), std::move(title),
        parent) {}

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(std::shared_ptr<Model> min,
      std::shared_ptr<Model> max, boost::optional<Type> default_min,
      boost::optional<Type> default_max, QString title, QWidget& parent)
      : QWidget(&parent),
        m_min(std::move(min)),
        m_max(std::move(max)),
        m_default_min(std::move(default_min)),
        m_default_max(std::move(default_max)) {
    m_filter_panel = new FilterPanel(std::move(title), this, parent);
    m_filter_panel->connect_reset_signal([=] { on_reset(); });
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    m_min_box = make_scalar_box(*m_min);
    auto min_layout = make_row_layout(tr("Min"), *m_min_box);
    layout->addLayout(min_layout);
    layout->addSpacing(scale_height(10));
    m_max_box = make_scalar_box(*m_max);
    auto max_layout = make_row_layout(tr("Max"), *m_max_box);
    layout->addLayout(max_layout);
    m_min_box->connect_submit_signal([=] (const auto& submission) {
      on_submit_min(submission);
    });
    m_max_box->connect_submit_signal([=] (const auto& submission) {
      on_submit_max(submission);
    });
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::Model>&
      ScalarFilterPanel<T>::get_min() const {
    return m_min;
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::Model>&
      ScalarFilterPanel<T>::get_max() const {
    return m_max;
  }

  template<typename T>
  const boost::optional<typename ScalarFilterPanel<T>::Type>&
      ScalarFilterPanel<T>::get_default_min() const {
    return m_default_min;
  }

  template<typename T>
  const boost::optional<typename ScalarFilterPanel<T>::Type>&
      ScalarFilterPanel<T>::get_default_max() const {
    return m_default_max;
  }

  template<typename T>
  void ScalarFilterPanel<T>::set_defaults(
      const boost::optional<Type>& default_min,
      const boost::optional<Type>& default_max) {
    m_default_min = default_min;
    m_default_max = default_max;
  }

  template<typename T>
  boost::signals2::connection ScalarFilterPanel<T>::connect_submit_signal(
      typename const SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  bool ScalarFilterPanel<T>::event(QEvent* event) {
    if(event->type() == QEvent::ShowToParent) {
      m_filter_panel->show();
      focusNextChild();
    }
    return QWidget::event(event);
  }

  template<typename T>
  typename ScalarFilterPanel<T>::ScalarBox*
      ScalarFilterPanel<T>::make_scalar_box(const Model& model) {
    auto field = [&] {
      if constexpr(std::is_same_v<ScalarBox, DurationBox>) {
        auto box = new ScalarBox();
        box->setFixedSize(scale(132, 26));
        return box;
      } else {
        auto modifiers = QHash<Qt::KeyboardModifier, Type>(
          {{Qt::NoModifier, model.get_increment()},
          {Qt::AltModifier, 5 * model.get_increment()},
          {Qt::ControlModifier, 10 * model.get_increment()},
          {Qt::ShiftModifier, 20 * model.get_increment()}});
        auto box =  new ScalarBox(std::move(modifiers));
        box->setFixedSize(scale(120, 26));
        return box;
      }
    }();
    field->get()->set_current(model.get());
    return field;
  }

  template<typename T>
  QHBoxLayout* ScalarFilterPanel<T>::make_row_layout(
      QString label, ScalarBox& box) {
    auto layout = new QHBoxLayout();
    auto label_box = new TextBox(std::move(label));
    label_box->set_read_only(true);
    label_box->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(label_box);
    layout->addSpacing(scale_width(18));
    layout->addStretch();
    layout->addWidget(&box);
    return layout;
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_reset() {
    m_min->set_current(m_default_min);
    m_max->set_current(m_default_max);
    m_min_box->get()->set_current(m_default_min);
    m_max_box->get()->set_current(m_default_max);
    m_submit_signal(m_default_min, m_default_max);
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_submit_min(
      const boost::optional<Type>& submission) {
    m_min->set_current(submission);
    if(m_max->get() && submission &&
        *m_max->get() < *submission) {
      m_max->set_current(submission);
      m_max_box->get()->set_current(submission);
    }
    m_submit_signal(m_min->get(), m_max->get());
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_submit_max(
      const boost::optional<Type>& submission) {
    m_max->set_current(submission);
    if(m_min->get() && submission &&
        *m_min->get() > *submission) {
      m_min->set_current(submission);
      m_min_box->get()->set_current(submission);
    }
    m_submit_signal(m_min->get(), m_max->get());
  }

  using DecimalFilterPanel = ScalarFilterPanel<DecimalBox>;
  using DurationFilterPanel = ScalarFilterPanel<DurationBox>;
  using IntegerFilterPanel = ScalarFilterPanel<IntegerBox>;
  using MoneyFilterPanel = ScalarFilterPanel<MoneyBox>;
  using QuantityFilterPanel = ScalarFilterPanel<QuantityBox>;
}

#endif
