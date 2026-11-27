#ifndef SPIRE_SUM_TABLE_AGGREGATOR_HPP
#define SPIRE_SUM_TABLE_AGGREGATOR_HPP
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
    double get() const;
  };

  std::any integer_sum(const boost::multiprecision::int128_t& value);
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
      std::conditional_t<std::integral<T>, boost::multiprecision::int128_t,
        Details::RealSum> m_sum = {};
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
      return Details::integer_sum(m_sum);
    } else if constexpr(std::floating_point<T>) {
      return m_sum.get();
    } else {
      return T(Nexus::Quantity::from_representation(m_sum.get()));
    }
  }

  template<typename T>
  void SumTableAggregator<T>::apply(const AnyRef& value, int count) {
    if(!value.has_value()) {
      return;
    }
    auto& item = any_cast<T>(value);
    if constexpr(std::integral<T>) {
      m_sum += boost::multiprecision::int128_t(item) * count;
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
