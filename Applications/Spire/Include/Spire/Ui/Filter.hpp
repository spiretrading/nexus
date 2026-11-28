#ifndef SPIRE_FILTER_HPP
#define SPIRE_FILTER_HPP
#include "Spire/Ui/ItemFilter.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"

namespace Spire {

  /**
   * Provides the standard item filter for a type.
   * @tparam T The type to filter.
   */
  template<typename T>
  class Filter;

  /**
   * Constructs a standard item filter.
   * @tparam T The type to filter.
   * @tparam Args The constructor argument types.
   * @param args Arguments forwarded to the filter's constructor.
   */
  template<typename T, typename... Args>
  auto filter(Args&&... args) {
    return std::make_unique<Filter<T>>(std::forward<Args>(args)...);
  }

  /** Filters integers using inclusive bounds. */
  template<>
  class Filter<int> : public ItemFilter {
    public:
      using Range = IntegerFilterPanel::Range;
      using RangeModel = IntegerFilterPanel::RangeModel;

      /** Constructs a filter with no bounds. */
      Filter();

      /**
       * Constructs a filter over a range model.
       * @param current The filtering bounds.
       */
      explicit Filter(std::shared_ptr<RangeModel> current) noexcept;

      /** Returns the filtering bounds. */
      const std::shared_ptr<RangeModel>& get_current() const;

      bool is_active() const override;
      bool is_filtered(const AnyRef& value) const override;
      QWidget* make_widget(QWidget& parent) override;
      void reset() override;
      boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<RangeModel> m_current;

      static void on_current(
        const FilterSignal::slot_type& slot, const Range& range);
  };
}

#endif
