#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
#include <functional>
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Represents an expression performed on a style. */
  template<typename T>
  class Expression {
    public:

      /**
       * Constructs an Expression evaluating to a constant.
       * @param constant The constant to represent.
       */
      Expression(T constant);

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

      bool operator !=(const Expression& expression) const;

    private:
      std::any m_value;
      std::function<bool (const Expression&, const Expression&)> m_is_equal;
  };

  template<typename T>
  Expression<T>::Expression(T constant)
    : m_value(std::move(constant)),
      m_is_equal([] (const Expression& left, const Expression& right) {
        if(left.m_value.type() != right.m_value.type()) {
          return false;
        }
        return left.as<T>() == right.as<T>();
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
}

#endif
