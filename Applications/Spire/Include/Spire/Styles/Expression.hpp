#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
#include <concepts>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/Evaluator.hpp"

namespace Spire::Styles {
  class Stylist;

  /**
   * Determines whether a type can be evaluated as an expression via the
   * make_evaluator function.
   */
  template<typename T>
  concept IsExpression = requires(T expression, const Stylist& stylist) {
    { make_evaluator(expression, stylist) };
  };

  /** Returns the type that an expression evaluates to. */
  template<typename T>
  struct expression_type {
    using type = T;
  };

  template<IsExpression T>
  struct expression_type<T> {
    using type = typename std::remove_cvref_t<T>::Type;
  };

  template<typename T>
  using expression_type_t =
    typename expression_type<std::remove_cvref_t<T>>::type;

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

      template<IsExpression E>
      Expression(E expression);

      /** Returns the type of the underlying expression. */
      const std::type_info& get_type() const;

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

    private:
      template<typename U>
      friend Evaluator<typename Expression<U>::Type>
        make_evaluator(const Expression<U>& expression, const Stylist& stylist);
      friend struct std::hash<Expression>;
      struct Operations {
        std::function<bool (const Expression&, const Expression&)> m_is_equal;
        std::function<Evaluator<Type> (const Expression&, const Stylist&)>
          m_make_evaluator;
        std::function<std::size_t (const Expression&)> m_hash;
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

  template<typename T>
  Expression<T>::Expression(Type value)
    : Expression(ConstantExpression(std::move(value))) {}

  template<typename T>
  template<IsExpression E>
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
        },
        [] (const Expression& expression) {
          return std::hash<E>()(expression.as<E>());
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
  Evaluator<typename Expression<T>::Type>
      Expression<T>::make_evaluator(const Stylist& stylist) const {
    auto& operations = m_operations.at(m_expression.type());
    return operations.m_make_evaluator(*this, stylist);
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::Expression<T>> {
    std::size_t operator ()(
        const Spire::Styles::Expression<T>& expression) const noexcept {
      auto& operations =
        Spire::Styles::Expression<T>::m_operations.at(expression.get_type());
      auto seed = std::size_t(0);
      boost::hash_combine(
        seed, std::hash<std::type_index>()(expression.get_type()));
      boost::hash_combine(seed, operations.m_hash(expression));
      return seed;
    }
  };
}

#endif
