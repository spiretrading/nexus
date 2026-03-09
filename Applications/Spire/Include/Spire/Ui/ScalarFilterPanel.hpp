#ifndef SPIRE_SCALAR_FILTER_PANEL_HPP
#define SPIRE_SCALAR_FILTER_PANEL_HPP
#include <memory>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
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
      };

      /** The type of model over the permissible range of values. */
      using RangeModel = ValueModel<Range>;

      /**
       * Constructs a ScalarFilterPanel.
       * @param range The range of permissible values.
       * @param parent The parent widget that shows the panel.
       */
      explicit ScalarFilterPanel(
        std::shared_ptr<RangeModel> current, QWidget* parent = nullptr);

      /** Returns the permissible range of values. */
      const std::shared_ptr<RangeModel>& get_current() const;

    private:
      std::shared_ptr<RangeModel> m_current;
      Range m_default_range;

      static TextBox* make_label_box(const QString& label);
      static ScalarBox* make_scalar_box(
        std::shared_ptr<ScalarValueModel<boost::optional<Type>>> current);
      void on_reset();
  };

  template<typename T>
  ScalarFilterPanel<T>::ScalarFilterPanel(
      std::shared_ptr<RangeModel> current, QWidget* parent)
      : QWidget(parent),
        m_current(std::move(current)),
        m_default_range(m_current->get()) {
    auto container = new QWidget();
    auto layout = make_hbox_layout(container);
    layout->addWidget(make_label_box(tr("Min")));
    auto min_box = make_scalar_box(make_scalar_value_model_decorator(
      make_field_value_model(m_current, &Range::m_min)));
    layout->addWidget(min_box, 1);
    layout->addSpacing(scale_width(18));
    layout->addWidget(make_label_box(tr("Max")));
    auto max_box = make_scalar_box(make_scalar_value_model_decorator(
      make_field_value_model(m_current, &Range::m_max)));
    layout->addWidget(max_box, 1);
    layout->addStretch();
    container->setMinimumWidth(layout->minimumSize().width());
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    enclose(*body, *container);
    auto panel = new FilterPanel(*body);
    panel->connect_reset_signal(
      std::bind_front(&ScalarFilterPanel::on_reset, this));
    enclose(*this, *panel);
    Styles::proxy_style(*this, *panel);
  }

  template<typename T>
  const std::shared_ptr<typename ScalarFilterPanel<T>::RangeModel>&
      ScalarFilterPanel<T>::get_current() const {
    return m_current;
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

  template<typename T>
  void ScalarFilterPanel<T>::on_reset() {
    m_current->set(m_default_range);
  }

  using DecimalFilterPanel = ScalarFilterPanel<DecimalBox>;
  using DurationFilterPanel = ScalarFilterPanel<DurationBox>;
  using IntegerFilterPanel = ScalarFilterPanel<IntegerBox>;
  using MoneyFilterPanel = ScalarFilterPanel<MoneyBox>;
  using QuantityFilterPanel = ScalarFilterPanel<QuantityBox>;
}

#endif
