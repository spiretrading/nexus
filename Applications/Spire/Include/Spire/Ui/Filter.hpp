#ifndef SPIRE_FILTER_HPP
#define SPIRE_FILTER_HPP
#include "Spire/Ui/ScalarFilter.hpp"

namespace Spire {

  /**
   * Selects the standard item filter for a type.
   * @tparam T The type to filter.
   */
  template<typename T>
  struct filter_type;

  /**
   * The standard item filter for a type.
   * @tparam T The type to filter.
   */
  template<typename T>
  using Filter = typename filter_type<T>::type;

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

  /** Filters durations using inclusive bounds. */
  template<>
  struct filter_type<boost::posix_time::time_duration> {

    /** The filter used for durations. */
    using type = ScalarFilter<DurationBox>;
  };

  /** Filters decimals using inclusive bounds. */
  template<>
  struct filter_type<Decimal> {

    /** The filter used for decimals. */
    using type = ScalarFilter<DecimalBox>;
  };

  /** Filters integers using inclusive bounds. */
  template<>
  struct filter_type<int> {

    /** The filter used for integers. */
    using type = ScalarFilter<IntegerBox>;
  };

  /** Filters money using inclusive bounds. */
  template<>
  struct filter_type<Nexus::Money> {

    /** The filter used for money. */
    using type = ScalarFilter<MoneyBox>;
  };

  /** Filters quantities using inclusive bounds. */
  template<>
  struct filter_type<Nexus::Quantity> {

    /** The filter used for quantities. */
    using type = ScalarFilter<QuantityBox>;
  };
}

#endif
