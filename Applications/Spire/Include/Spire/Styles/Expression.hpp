#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/Evaluator.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
  template<typename T, typename = void>
  struct is_expression_t : std::false_type {};

  template<typename T>
  struct is_expression_t<Expression<T>> : std::true_type {};

  template<typename T>
  struct is_expression_t<T, std::void_t<decltype(
    make_evaluator(std::declval<T>(), std::declval<const Stylist&>()))>> :
      std::true_type {};

  template<typename T>
  constexpr auto is_expression_v = is_expression_t<T>::value;

  template<typename T, typename = void>
  struct expression_type {
    using type = T;
  };

  template<typename T>
  struct
      expression_type<T, std::enable_if_t<is_expression_v<std::decay_t<T>>>> {
    using type = typename T::Type;
  };

  template<typename T>
  using expression_type_t = typename expression_type<std::decay_t<T>>::type;

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

      template<typename E, typename = std::enable_if_t<is_expression_v<E>>>
      Expression(E expression);

      /** Returns the type of the underlying expression. */
      const std::type_info& get_type() const;

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

      bool operator !=(const Expression& expression) const;

    private:
      template<typename U>
      friend Evaluator<typename Expression<U>::Type>
        make_evaluator(const Expression<U>& expression, const Stylist& stylist);
      struct Operations {
        std::function<bool (const Expression&, const Expression&)> m_is_equal;
        std::function<Evaluator<Type> (const Expression&, const Stylist&)>
          m_make_evaluator;
      };
      static inline std::unordered_map<std::type_index, Operations>
        m_operations;
      std::any m_expression;

      Evaluator<Type> make_evaluator(const Stylist& stylist) const;
  };

  /**
   * Returns a function that can be used to evaluate an Expression.
   * @param expression The expression to evaluate.
   * @param stylist The Stylist is used to provide context to the evaluator.
   * @return A function that can be used to evaluate the <i>expression</i>.
   */
  template<typename T>
  Evaluator<typename Expression<T>::Type> make_evaluator(
      const Expression<T>& expression, const Stylist& stylist) {
    return expression.make_evaluator(stylist);
  }

  template<typename E, typename = std::enable_if_t<is_expression_v<E>>>
  Evaluator<expression_type_t<E>> make_evaluator(
      const E& expression, const Stylist& stylist) {
    return make_evaluator(expression, stylist);
  }

  template<typename T>
  Expression<T>::Expression(Type value)
    : Expression(ConstantExpression(std::move(value))) {}

  template<typename T>
  template<typename E, typename>
  Expression<T>::Expression(E expression)
      : m_expression(std::move(expression)) {
    auto operations = m_operations.find(typeid(E));
    if(operations == m_operations.end()) {
      m_operations.emplace_hint(operations, typeid(E), Operations(
        [] (const Expression& left, const Expression& right) {
          return left.m_expression.type() == right.m_expression.type() &&
            left.as<E>() == right.as<E>();
        },
        [] (const Expression& expression, const Stylist& stylist) {
          return Spire::Styles::make_evaluator(expression.as<E>(), stylist);
        }));
    }
  }

  template<typename T>
  const std::type_info& Expression<T>::get_type() const {
    return m_expression.type();
  }

  template<typename T>
  template<typename U>
  const U& Expression<T>::as() const {
    return std::any_cast<const U&>(m_expression);
  }

  template<typename T>
  bool Expression<T>::operator ==(const Expression& expression) const {
    auto& operations = m_operations.at(m_expression.type());
    return operations.m_is_equal(*this, expression);
  }

  template<typename T>
  bool Expression<T>::operator !=(const Expression& expression) const {
    return !(*this == expression);
  }

  template<typename T>
  Evaluator<typename Expression<T>::Type>
      Expression<T>::make_evaluator(const Stylist& stylist) const {
    auto& operations = m_operations.at(m_expression.type());
    return operations.m_make_evaluator(*this, stylist);
  }
}

#endif
