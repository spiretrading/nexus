#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
#include <functional>
#include <tuple>
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Represents an expression performed on a style.
   * @param <T> The type the expression evaluates to.
   */
  template<typename T>
  class Expression {
    public:

      /** The type the expression evaluates to. */
      using Type = T;

      /**
       * Constructs an Expression evaluating to a constant.
       * @param constant The constant to represent.
       */
      Expression(Type constant);

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

      bool operator !=(const Expression& expression) const;

    private:
      std::any m_value;
      std::function<bool (const Expression&, const Expression&)> m_is_equal;
  };

  /**
   * Provides a helper template class for a generic expression definition.
   * @param <I> A unique struct ID/tag used to differentiate definitions.
   * @param <T> The type the expression evaluates to.
   * @param <A> The type of the expression's arguments.
   */
  template<typename I, typename T, typename... A>
  class ExpressionDefinition {
    public:

      /** The type the expression evaluates to. */
      using Type = T;

      /** The tuple of arguments. */
      using Arguments = std::tuple<A...>;

      /**
       * Constructs an ExpressionDefinition.
       * @param arguments The arguments used to evaluate the expression.
       */
      explicit ExpressionDefinition(Expression<A>... arguments);

      /** Returns the tuple of arguments. */
      const Arguments& get_arguments() const;

    private:
      Arguments m_arguments;
  };

  template<typename T>
  Expression<T>::Expression(Type constant)
    : m_value(std::move(constant)),
      m_is_equal([] (const Expression& left, const Expression& right) {
        if(left.m_value.type() != right.m_value.type()) {
          return false;
        }
        return left.as<Type>() == right.as<Type>();
      }) {}

  template<typename T>
  template<typename U>
  const U& Expression<T>::as() const {
    return std::any_cast<const U&>(m_value);
  }

  template<typename T>
  bool Expression<T>::operator ==(const Expression& expression) const {
    return m_is_equal(*this, expression);
  }

  template<typename T>
  bool Expression<T>::operator !=(const Expression& expression) const {
    return !(*this == expression);
  }

  template<typename I, typename T, typename... A>
  ExpressionDefinition<I, T, A...>::ExpressionDefinition(
    Expression<A>... arguments)
    : m_arguments(std::move(arguments)...) {}

  template<typename I, typename T, typename... A>
  const typename ExpressionDefinition<I, T, A...>::Arguments&
      ExpressionDefinition<I, T, A...>::get_arguments() const {
    return m_arguments;
  }
}

#endif
