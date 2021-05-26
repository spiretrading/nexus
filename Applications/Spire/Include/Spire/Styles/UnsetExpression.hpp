#ifndef SPIRE_STYLES_UNSET_EXPRESSION_HPP
#define SPIRE_STYLES_UNSET_EXPRESSION_HPP
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Reverts a property's value as if it had not been set by the current rule.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class UnsetExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      bool operator ==(const UnsetExpression& expression) const;

      bool operator !=(const UnsetExpression& expression) const;
  };

  /**
   * This is used to define a single variable that can polymorphically
   * substitute for any UnsetExpression<T> without explicitly specifying the
   * type.
   */
  struct UnsetPolymorph {
    template<typename T>
    operator UnsetExpression<T>() const;

    template<typename T>
    operator Expression<T>() const;
  };

  /**
   * Provides a single 'keyword' like variable that can be used to unset
   * a property.
   */
  constexpr auto unset = UnsetPolymorph();

  template<typename T>
  Evaluator<T>
      make_evaluator(UnsetExpression<T> expression, const Stylist& stylist) {
    return stylist.unset<T>();
  }

  template<typename T>
  bool UnsetExpression<T>::operator ==(
      const UnsetExpression& expression) const {
    return true;
  }

  template<typename T>
  bool UnsetExpression<T>::operator !=(
      const UnsetExpression& expression) const {
    return !(*this == expression);
  }

  template<typename T>
  UnsetPolymorph::operator UnsetExpression<T>() const {
    return UnsetExpression<T>();
  }

  template<typename T>
  UnsetPolymorph::operator Expression<T>() const {
    return Expression<T>(UnsetExpression<T>());
  }
}

#endif
