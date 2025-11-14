#ifndef SPIRE_STYLES_CONSTANT_EXPRESSION_HPP
#define SPIRE_STYLES_CONSTANT_EXPRESSION_HPP
#include <functional>
#include "Spire/Spire/Hash.hpp"
#include "Spire/Styles/Evaluator.hpp"

namespace Spire::Styles {
  class Stylist;

  /**
   * Expresses a constant value.
   * @param <T> The constant's data type.
   */
  template<typename T>
  class ConstantExpression {
    public:

      /** The constant's type. */
      using Type = T;

      /**
       * Constructs a ConstantExpression.
       * @param constant The value to evaluate to.
       */
      explicit ConstantExpression(Type constant);

      /** Returns the constant. */
      const Type& get_constant() const &;

      /** Returns the constant. */
      Type&& get_constant() &&;

      bool operator ==(const ConstantExpression& expression) const = default;

    private:
      Type m_constant;
  };

  template<typename T>
  auto make_evaluator(
      ConstantExpression<T> expression, const Stylist& stylist) {
    return [constant = std::move(expression).get_constant()] (
        boost::posix_time::time_duration) {
      return Evaluation(constant);
    };
  }

  template<typename T>
  ConstantExpression<T>::ConstantExpression(Type constant)
    : m_constant(std::move(constant)) {}

  template<typename T>
  const typename ConstantExpression<T>::Type&
      ConstantExpression<T>::get_constant() const & {
    return m_constant;
  }

  template<typename T>
  typename ConstantExpression<T>::Type&&
      ConstantExpression<T>::get_constant() && {
    return std::move(m_constant);
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::ConstantExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::ConstantExpression<T>& expression) const noexcept {
      return std::hash<T>()(expression.get_constant());
    }
  };
}

#endif
