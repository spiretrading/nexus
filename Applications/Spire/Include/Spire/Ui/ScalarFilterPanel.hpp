#ifndef SPIRE_SCALAR_FILTER_PANEL_HPP
#define SPIRE_SCALAR_FILTER_PANEL_HPP
#include <memory>
#include <type_traits>
#include <QEvent>
#include <QWidget>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/TextBox.hpp"

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
        decltype(*std::declval<ScalarBox>().get_current())>::Scalar;

      /** Stores the range of permissible values. */
      struct Range {

        /** The minimum permissible value (inclusive). */
        boost::optional<Type> m_min;

        /** The maximum permissible value (inclusive). */
        boost::optional<Type> m_max;

        bool operator ==(const Range&) const = default;
      };

      /** The type of model over the permissible range of values. */
      using RangeModel = ValueModel<Range>;

      /**
       * Signals a submission to the range of values.
       * @param submission The submitted range.
       */
      using SubmitSignal = Signal<void (const Range& submission)>;

      /**
       * Constructs a ScalarFilterPanel using a local model representing an
       * unbounded range.
       * @param title The title of the panel.
       * @param parent The parent widget showing the panel.
       */
      ScalarFilterPanel(QString title, QWidget& parent);

      /**
       * Constructs a ScalarFilterPanel.
       * @param range The range of permissible values.
       * @param title The title of the panel.
       * @param parent The parent widget that shows the panel.
       */
      ScalarFilterPanel(
        std::shared_ptr<RangeModel> range, QString title, QWidget& parent);

      /** Returns the permissible range of values. */
      const std::shared_ptr<RangeModel>& get_range() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<RangeModel> m_range;
      FilterPanel* m_filter_panel;
      ScalarBox* m_min_box;
      ScalarBox* m_max_box;

      static ScalarBox* make_scalar_box(const boost::optional<Type>& current);
      static QHBoxLayout* make_row_layout(QString label, ScalarBox& box);
      void on_reset();
      void on_submit_min(const boost::optional<Type>& submission);
      void on_submit_max(const boost::optional<Type>& submission);
  };

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(QString title, QWidget& parent)
    : ScalarFilterPanel(std::make_shared<LocalValueModel<Range>>(),
        std::move(title), parent) {}

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(
      std::shared_ptr<RangeModel> range, QString title, QWidget& parent)
      : QWidget(&parent),
        m_range(std::move(range)) {
    m_filter_panel = new FilterPanel(std::move(title), this, parent);
    m_filter_panel->connect_reset_signal([=] { on_reset(); });
    m_min_box = make_scalar_box(m_range->get().m_min);
    auto min_layout = make_row_layout(tr("Min"), *m_min_box);
    auto layout = make_vbox_layout(this);
    layout->addLayout(min_layout);
    layout->addSpacing(scale_height(10));
    m_max_box = make_scalar_box(m_range->get().m_max);
    auto max_layout = make_row_layout(tr("Max"), *m_max_box);
    layout->addLayout(max_layout);
    m_min_box->connect_submit_signal(
      std::bind_front(&ScalarFilterPanel::on_submit_min, this));
    m_max_box->connect_submit_signal(
      std::bind_front(&ScalarFilterPanel::on_submit_max, this));
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::RangeModel>&
      ScalarFilterPanel<T>::get_range() const {
    return m_range;
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
      ScalarFilterPanel<T>::make_scalar_box(
        const boost::optional<Type>& current) {
    auto field = [&] {
      auto box = new ScalarBox();
      auto width = [] {
        if constexpr(std::is_same_v<ScalarBox, DurationBox>) {
          return 132;
        } else {
          return 120;
        }
      }();
      box->setFixedSize(scale(width, 26));
      return box;
    }();
    field->get_current()->set(current);
    return field;
  }

  template<typename T>
  QHBoxLayout* ScalarFilterPanel<T>::make_row_layout(
      QString label, ScalarBox& box) {
    auto label_box = new TextBox(std::move(label));
    label_box->set_read_only(true);
    label_box->setFocusPolicy(Qt::NoFocus);
    auto layout = make_hbox_layout();
    layout->addWidget(label_box);
    layout->addSpacing(scale_width(18));
    layout->addStretch();
    layout->addWidget(&box);
    return layout;
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_reset() {
    m_range->set({boost::none, boost::none});
    m_submit_signal({boost::none, boost::none});
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_submit_min(
      const boost::optional<Type>& submission) {
    auto range = Range(submission, m_range->get().m_max);
    if(range.m_max && submission && *range.m_max < *submission) {
      range.m_max = submission;
      m_max_box->get_current()->set(submission);
    }
    m_range->set(range);
    m_submit_signal(range);
  }

  template<typename T>
  void ScalarFilterPanel<T>::on_submit_max(
      const boost::optional<Type>& submission) {
    auto range = Range(m_range->get().m_min, submission);
    if(range.m_min && submission && *range.m_min > *submission) {
      range.m_min = submission;
      m_min_box->get_current()->set(submission);
    }
    m_range->set(range);
    m_submit_signal(range);
  }

  using DecimalFilterPanel = ScalarFilterPanel<DecimalBox>;
  using DurationFilterPanel = ScalarFilterPanel<DurationBox>;
  using IntegerFilterPanel = ScalarFilterPanel<IntegerBox>;
  using MoneyFilterPanel = ScalarFilterPanel<MoneyBox>;
  using QuantityFilterPanel = ScalarFilterPanel<QuantityBox>;
}

#endif
