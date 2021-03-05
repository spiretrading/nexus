#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
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

    private:
      std::any m_value;
  };

  template<typename T>
  Expression<T>::Expression(T constant)
    : m_value(std::move(constant)) {}

  template<typename T>
  template<typename U>
  const U& Expression<T>::as() const {
    return std::any_cast<const U&>(m_value);
  }
}

#endif
