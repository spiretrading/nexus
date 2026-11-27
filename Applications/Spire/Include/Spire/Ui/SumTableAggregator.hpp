#ifndef SPIRE_SUM_TABLE_AGGREGATOR_HPP
#define SPIRE_SUM_TABLE_AGGREGATOR_HPP
#include <cstdint>
#include <limits>
#include <boost/multiprecision/cpp_int.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Ui/TableAggregator.hpp"

namespace Spire {
namespace Details {
  struct RealSum {
    boost::multiprecision::cpp_int m_significand = 0;
    int m_exponent = 0;
    int m_nan = 0;
    int m_positive_infinity = 0;
    int m_negative_infinity = 0;

    void apply(long double value, int count);
    template<std::floating_point T>
    T get() const;
  };
}

  /**
   * Maintains the sum of a numeric column, ignoring missing values.
   * @tparam T The column's underlying numeric type.
   */
  template<typename T>
  class SumTableAggregator final : public TableAggregator {
    public:
      void add(const AnyRef& value) override;
      void remove(const AnyRef& value) override;
      std::any get() const override;

    private:
      using IntegerSum = std::conditional_t<(sizeof(T) <= sizeof(std::int32_t)),
        std::int64_t, boost::multiprecision::int128_t>;
      std::conditional_t<std::integral<T>, IntegerSum, Details::RealSum>
        m_sum = {};
      int m_count = 0;

      void apply(const AnyRef& value, int count);
  };

  template<typename T>
  void SumTableAggregator<T>::add(const AnyRef& value) {
    apply(value, 1);
  }

  template<typename T>
  void SumTableAggregator<T>::remove(const AnyRef& value) {
    apply(value, -1);
  }

  template<typename T>
  std::any SumTableAggregator<T>::get() const {
    if(m_count == 0) {
      return {};
    }
    if constexpr(std::integral<T>) {
      if(m_sum < std::numeric_limits<T>::min() ||
          m_sum > std::numeric_limits<T>::max()) {
        return {};
      }
      return static_cast<T>(m_sum);
    } else if constexpr(std::floating_point<T>) {
      return m_sum.template get<T>();
    } else {
      return T(Nexus::Quantity::from_representation(
        m_sum.template get<double>()));
    }
  }

  template<typename T>
  void SumTableAggregator<T>::apply(const AnyRef& value, int count) {
    if(!value.has_value()) {
      return;
    }
    auto& item = any_cast<T>(value);
    if constexpr(std::integral<T>) {
      m_sum += IntegerSum(item) * count;
    } else if constexpr(std::floating_point<T>) {
      m_sum.apply(item, count);
    } else {
      m_sum.apply(
        static_cast<Nexus::Quantity>(item).get_representation(), count);
    }
    m_count += count;
  }
}

#endif
