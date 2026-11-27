#ifndef SPIRE_DISTINCT_TABLE_AGGREGATOR_HPP
#define SPIRE_DISTINCT_TABLE_AGGREGATOR_HPP
#include <cmath>
#include <unordered_map>
#include "Spire/Spire/Hash.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableAggregator.hpp"

namespace Spire {

  /**
   * Counts distinct column values, ignoring missing values.
   * @tparam T The column's underlying value type.
   */
  template<typename T>
  class DistinctTableAggregator final : public TableAggregator {
    public:
      void add(const AnyRef& value) override;
      void remove(const AnyRef& value) override;
      std::any get() const override;

    private:
      struct Hash {
        static constexpr auto IS_NOTHROW = [] {
          if constexpr(requires(const T& value) { std::hash<T>()(value); }) {
            return std::is_nothrow_default_constructible_v<std::hash<T>> &&
              std::is_nothrow_invocable_r_v<
                std::size_t, std::hash<T>, const T&>;
          } else {
            return requires(const T& value) {
              { hash_value(value) } noexcept;
              requires std::is_nothrow_convertible_v<
                decltype(hash_value(value)), std::size_t>;
            };
          }
        }();

        std::size_t operator ()(const T& value) const noexcept(IS_NOTHROW);
      };
      struct Equal {
        bool operator ()(const T& left, const T& right) const noexcept(
          noexcept(static_cast<bool>(left == right)));
      };
      std::unordered_map<T, int, Hash, Equal> m_frequencies;
  };

  template<typename T>
  std::size_t DistinctTableAggregator<T>::Hash::operator ()(
      const T& value) const noexcept(IS_NOTHROW) {
    if constexpr(std::floating_point<T>) {
      if(std::isnan(value)) {
        return 0;
      }
    }
    if constexpr(requires { std::hash<T>()(value); }) {
      return std::hash<T>()(value);
    } else if constexpr(requires {
        { hash_value(value) } -> std::convertible_to<std::size_t>;
      }) {
      return hash_value(value);
    } else {
      return Spire::hash_value(AnyRef(value)).value_or(0);
    }
  }

  template<typename T>
  bool DistinctTableAggregator<T>::Equal::operator ()(
      const T& left, const T& right) const noexcept(
        noexcept(static_cast<bool>(left == right))) {
    if constexpr(std::floating_point<T>) {
      return left == right || (std::isnan(left) && std::isnan(right));
    } else {
      return static_cast<bool>(left == right);
    }
  }

  template<typename T>
  void DistinctTableAggregator<T>::add(const AnyRef& value) {
    if(value.has_value()) {
      ++m_frequencies[any_cast<T>(value)];
    }
  }

  template<typename T>
  void DistinctTableAggregator<T>::remove(const AnyRef& value) {
    if(!value.has_value()) {
      return;
    }
    auto i = m_frequencies.find(any_cast<T>(value));
    if(i != m_frequencies.end()) {
      --i->second;
      if(i->second == 0) {
        m_frequencies.erase(i);
      }
    }
  }

  template<typename T>
  std::any DistinctTableAggregator<T>::get() const {
    if(m_frequencies.empty()) {
      return {};
    }
    return DistinctCount(std::ssize(m_frequencies));
  }
}

#endif
