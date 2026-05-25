#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/Evaluator.hpp"

namespace Spire::Styles {
  class Stylist;
  template<typename> class Expression;

namespace Details {
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
}

  /** Returns the type that an expression evaluates to. */
  template<typename T, typename = void>
  struct expression_type {
    using type = T;
  };

  template<typename T>
  struct expression_type<
      T, std::enable_if_t<Details::is_expression_v<std::remove_cvref_t<T>>>> {
    using type = typename T::Type;
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

      /*
       * Constructs an Expression from another expression.
       * @param expression The expression to represent.
       */
      template<typename E,
        typename = std::enable_if_t<Details::is_expression_v<E>>>
      Expression(E expression);

      Expression(const Expression&) = default;
      Expression(Expression&&) = default;

      /** Returns the type of the underlying expression. */
      std::type_index get_type() const;

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

    private:
      template<typename U>
      friend Evaluator<typename Expression<U>::Type>
        make_evaluator(const Expression<U>& expression, const Stylist& stylist);
      friend struct std::hash<Expression>;
      struct BaseHolder {
        virtual ~BaseHolder() = default;

        virtual std::type_index get_type() const = 0;
      };
      template<typename E>
      struct Holder final : BaseHolder {
        E m_value;

        Holder(E value);

        std::type_index get_type() const override;
      };
      struct Operations {
        bool (*m_is_equal)(const Expression&, const Expression&);
        Evaluator<Type> (*m_make_evaluator)(const Expression&, const Stylist&);
        std::size_t (*m_hash)(const Expression&);

        Operations(bool (*is_equal)(const Expression&, const Expression&),
          Evaluator<Type> (*make_evaluator)(const Expression&, const Stylist&),
          std::size_t (*hash)(const Expression&)) noexcept;
      };
      static inline std::unordered_map<std::type_index, Operations>
        m_operations;
      std::shared_ptr<const BaseHolder> m_holder;

      Evaluator<Type> make_evaluator(const Stylist& stylist) const;
      Expression& operator =(const Expression&) = delete;
      Expression& operator =(Expression&&) = delete;
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

  template<typename E, typename = std::enable_if_t<Details::is_expression_v<E>>>
  Evaluator<expression_type_t<E>> make_evaluator(
      const E& expression, const Stylist& stylist) {
    return make_evaluator(expression, stylist);
  }

  template<typename T>
  Expression<T>::Operations::Operations(
    bool (*is_equal)(const Expression&, const Expression&),
    Evaluator<Type> (*make_evaluator)(const Expression&, const Stylist&),
    std::size_t (*hash)(const Expression&)) noexcept
    : m_is_equal(is_equal),
      m_make_evaluator(make_evaluator),
      m_hash(hash) {}

  template<typename T>
  template<typename E>
  Expression<T>::Holder<E>::Holder(E value)
    : m_value(std::move(value)) {}

  template<typename T>
  template<typename E>
  std::type_index Expression<T>::Holder<E>::get_type() const {
    return typeid(E);
  }

  template<typename T>
  Expression<T>::Expression(Type value)
    : Expression(ConstantExpression(std::move(value))) {}

  template<typename T>
  template<typename E, typename>
  Expression<T>::Expression(E expression)
      : m_holder(std::make_shared<Holder<E>>(std::move(expression))) {
    auto operations = m_operations.find(typeid(E));
    if(operations == m_operations.end()) {
      m_operations.emplace_hint(operations, typeid(E), Operations(
        +[] (const Expression& left, const Expression& right) {
          return left.get_type() == right.get_type() &&
            left.as<E>() == right.as<E>();
        },
        +[] (const Expression& expression, const Stylist& stylist) ->
            Evaluator<Type> {
          return Spire::Styles::make_evaluator(expression.as<E>(), stylist);
        },
        +[] (const Expression& expression) {
          return std::hash<E>()(expression.as<E>());
        }));
    }
  }

  template<typename T>
  std::type_index Expression<T>::get_type() const {
    return m_holder->get_type();
  }

  template<typename T>
  template<typename U>
  const U& Expression<T>::as() const {
    return static_cast<const Holder<U>&>(*m_holder).m_value;
  }

  template<typename T>
  bool Expression<T>::operator ==(const Expression& expression) const {
    auto& operations = m_operations.at(m_holder->get_type());
    return operations.m_is_equal(*this, expression);
  }

  template<typename T>
  Evaluator<typename Expression<T>::Type>
      Expression<T>::make_evaluator(const Stylist& stylist) const {
    auto& operations = m_operations.at(m_holder->get_type());
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
