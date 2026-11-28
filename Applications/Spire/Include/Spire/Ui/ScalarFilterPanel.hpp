#ifndef SPIRE_SCALAR_FILTER_PANEL_HPP
#define SPIRE_SCALAR_FILTER_PANEL_HPP
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/PercentBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /**
   * Displays a FilterPanel over a range of scalar values.
   * @tparam T The type of component used to input the scalar values.
   */
  template<typename T>
  class ScalarFilterPanel : public QWidget {
    public:

      /** The type of component used to input the scalar values. */
      using ScalarBox = T;

      /** The type of scalar to filter. */
      using Type = typename std::remove_reference_t<
        decltype(*std::declval<ScalarBox>().get_current())>::Scalar;

      /** The model of an optional bound. */
      using BoundModel = ScalarValueModel<boost::optional<Type>>;

      /** Signals a request to reset the bounds. */
      using ResetSignal = FilterPanel::ResetSignal;

      /**
       * Constructs a ScalarFilterPanel.
       * @param minimum The minimum value model.
       * @param maximum The maximum value model.
       * @param parent The parent widget.
       */
      ScalarFilterPanel(std::shared_ptr<BoundModel> minimum,
        std::shared_ptr<BoundModel> maximum, QWidget* parent = nullptr);

      /** Returns the minimum value model. */
      const std::shared_ptr<BoundModel>& get_minimum() const;

      /** Returns the maximum value model. */
      const std::shared_ptr<BoundModel>& get_maximum() const;

      /** Connects a slot to reset requests. */
      boost::signals2::connection connect_reset_signal(
        const ResetSignal::slot_type& slot) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      ScalarBox* m_min_box;
      ScalarBox* m_max_box;
      FilterPanel* m_panel;

      static TextBox* make_label_box(const QString& label);
      static ScalarBox* make_scalar_box(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current);
  };

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(std::shared_ptr<BoundModel> minimum,
      std::shared_ptr<BoundModel> maximum, QWidget* parent)
      : QWidget(parent) {
    auto container = new QWidget();
    auto layout = make_hbox_layout(container);
    layout->addWidget(make_label_box(tr("Min")));
    m_min_box = make_scalar_box(std::move(minimum));
    layout->addWidget(m_min_box, 1);
    layout->addSpacing(scale_width(18));
    layout->addWidget(make_label_box(tr("Max")));
    m_max_box = make_scalar_box(std::move(maximum));
    layout->addWidget(m_max_box, 1);
    layout->addStretch();
    container->setMinimumWidth(layout->minimumSize().width());
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    enclose(*body, *container);
    m_panel = new FilterPanel(*body);
    enclose(*this, *m_panel);
    Styles::proxy_style(*this, *m_panel);
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::BoundModel>&
      ScalarFilterPanel<T>::get_minimum() const {
    return m_min_box->get_current();
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::BoundModel>&
      ScalarFilterPanel<T>::get_maximum() const {
    return m_max_box->get_current();
  }

  template<typename T>
  boost::signals2::connection ScalarFilterPanel<T>::connect_reset_signal(
      const ResetSignal::slot_type& slot) const {
    return m_panel->connect_reset_signal(slot);
  }

  template<typename T>
  void ScalarFilterPanel<T>::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_min_box->setFocus();
  }

  template<typename T>
  TextBox* ScalarFilterPanel<T>::make_label_box(const QString& label) {
    auto box = make_label(label);
    box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    Styles::update_style(*box, [] (auto& style) {
      style.get(Styles::Any()).set(Styles::PaddingRight(scale_width(8)));
    });
    return box;
  }

  template<typename T>
  typename ScalarFilterPanel<T>::ScalarBox*
      ScalarFilterPanel<T>::make_scalar_box(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current) {
    auto box = new ScalarBox(std::move(current));
    if constexpr(std::is_same_v<ScalarBox, DurationBox>) {
      box->setFixedWidth(box->layout()->minimumSize().width());
    } else {
      box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      box->setMinimumWidth(scale_width(80));
      box->setMaximumWidth(scale_width(120));
    }
    return box;
  }

  using DecimalFilterPanel = ScalarFilterPanel<DecimalBox>;
  using DurationFilterPanel = ScalarFilterPanel<DurationBox>;
  using IntegerFilterPanel = ScalarFilterPanel<IntegerBox>;
  using MoneyFilterPanel = ScalarFilterPanel<MoneyBox>;
  using PercentFilterPanel = ScalarFilterPanel<PercentBox>;
  using QuantityFilterPanel = ScalarFilterPanel<QuantityBox>;
}

#endif
