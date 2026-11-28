#ifndef SPIRE_SCALAR_FILTER_HPP
#define SPIRE_SCALAR_FILTER_HPP
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/ItemFilter.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"

namespace Spire {

  /**
   * Filters scalar values using inclusive bounds.
   * @tparam T The input box used to edit the bounds.
   */
  template<typename T>
  class ScalarFilter : public ItemFilter {
    public:

      /** The input box used to edit the bounds. */
      using ScalarBox = T;

      /** The type of scalar to filter. */
      using Type = typename ScalarFilterPanel<ScalarBox>::Type;

      /** Stores the range of permissible values. */
      struct Range {

        /** The minimum permissible value (inclusive). */
        boost::optional<Type> m_min;

        /** The maximum permissible value (inclusive). */
        boost::optional<Type> m_max;

        bool operator ==(const Range&) const = default;
      };

      /** The model of the filtering bounds. */
      using RangeModel = ValueModel<Range>;

      /** Constructs a filter with no bounds. */
      ScalarFilter();

      /**
       * Constructs a filter with initial bounds.
       * @param range The filtering bounds.
       */
      explicit ScalarFilter(Range range);

      /**
       * Constructs a filter over a range model.
       * @param current The filtering bounds.
       */
      explicit ScalarFilter(std::shared_ptr<RangeModel> current) noexcept;

      /** Returns the filtering bounds. */
      const std::shared_ptr<RangeModel>& get_current() const;

      /**
       * Creates controls for editing the filtering bounds.
       * @param parent The parent widget.
       */
      QWidget* make_widget(QWidget* parent = nullptr);

      bool is_active() const override;
      bool is_filtered(const AnyRef& value) const override;
      QWidget* make_widget(QWidget& parent) override;
      void reset() override;
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<RangeModel> m_current;

      static void on_reset(const std::shared_ptr<RangeModel>& current);
      static void on_current(
        const FilterSignal::slot_type& slot, const Range& range);
  };

  template<typename T>
  ScalarFilter<T>::ScalarFilter()
    : ScalarFilter(Range()) {}

  template<typename T>
  ScalarFilter<T>::ScalarFilter(Range range)
    : ScalarFilter(
        std::make_shared<LocalValueModel<Range>>(std::move(range))) {}

  template<typename T>
  ScalarFilter<T>::ScalarFilter(std::shared_ptr<RangeModel> current) noexcept
    : m_current(std::move(current)) {}

  template<typename T>
  const std::shared_ptr<typename ScalarFilter<T>::RangeModel>&
      ScalarFilter<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  QWidget* ScalarFilter<T>::make_widget(QWidget* parent) {
    auto panel = new ScalarFilterPanel<ScalarBox>(
      make_scalar_value_model_decorator(
        make_field_value_model(m_current, &Range::m_min)),
      make_scalar_value_model_decorator(
        make_field_value_model(m_current, &Range::m_max)), parent);
    panel->connect_reset_signal(
      std::bind_front(&ScalarFilter::on_reset, m_current));
    return panel;
  }

  template<typename T>
  bool ScalarFilter<T>::is_active() const {
    auto& range = m_current->get();
    return range.m_min || range.m_max;
  }

  template<typename T>
  bool ScalarFilter<T>::is_filtered(const AnyRef& value) const {
    auto& item = any_cast<Type>(value);
    auto& range = m_current->get();
    return range.m_min && item < *range.m_min ||
      range.m_max && item > *range.m_max;
  }

  template<typename T>
  QWidget* ScalarFilter<T>::make_widget(QWidget& parent) {
    return make_widget(&parent);
  }

  template<typename T>
  void ScalarFilter<T>::reset() {
    on_reset(m_current);
  }

  template<typename T>
  boost::signals2::connection ScalarFilter<T>::connect_filter_signal(
      const FilterSignal::slot_type& slot) const {
    return m_current->connect_update_signal(
      std::bind_front(&ScalarFilter::on_current, slot));
  }

  template<typename T>
  void ScalarFilter<T>::on_reset(const std::shared_ptr<RangeModel>& current) {
    current->set(Range());
  }

  template<typename T>
  void ScalarFilter<T>::on_current(
      const FilterSignal::slot_type& slot, const Range&) {
    slot();
  }
}

#endif
