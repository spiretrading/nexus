#ifndef SPIRE_STYLES_BASIC_PROPERTY_HPP
#define SPIRE_STYLES_BASIC_PROPERTY_HPP
#include <utility>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Represents the declaration of a style property from an Expression.
   * @param <T> The property's type.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class BasicProperty {
    public:

      /** The type of the property's value. */
      using Type = T;

      /** A unique tag used to identify the property. */
      using Tag = G;

      /** The type of Expression assigned to this property. */
      using Expression = Spire::Styles::Expression<Type>;

      /**
       * Constructs a BasicProperty assigned to an Expression.
       * @param expression The Expression assigned to this property.
       */
      BasicProperty(Expression expression);

      /**
       * Constructs a BasicProperty assigned to a ConstantExpression.
       * @param value The value used to construct the ConstantExpression.
       */
      BasicProperty(Type value);

      /** Returns the property's Expression. */
      const Expression& get_expression() const;

      bool operator ==(const BasicProperty& property) const;

      bool operator !=(const BasicProperty& property) const;

    private:
      Expression m_expression;
  };

  template<typename T, typename G>
  BasicProperty<T, G>::BasicProperty(Expression expression)
    : m_expression(std::move(expression)) {}

  template<typename T, typename G>
  BasicProperty<T, G>::BasicProperty(Type value)
    : BasicProperty(ConstantExpression(std::move(value))) {}

  template<typename T, typename G>
  const typename BasicProperty<T, G>::Expression&
      BasicProperty<T, G>::get_expression() const {
    return m_expression;
  }

  template<typename T, typename G>
  bool BasicProperty<T, G>::operator ==(const BasicProperty& property) const {
    return m_expression == property.get_expression();
  }

  template<typename T, typename G>
  bool BasicProperty<T, G>::operator !=(const BasicProperty& property) const {
    return !(*this == property);
  }
}

#endif
