#ifndef SPIRE_STYLES_REVERT_EXPRESSION_HPP
#define SPIRE_STYLES_REVERT_EXPRESSION_HPP
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/Stylist.hpp"

namespace Spire::Styles {

  /**
   * Reverts a property's value as if it had not been set by the current rule.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class RevertExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      bool operator ==(const RevertExpression& expression) const = default;
  };

  /**
   * This is used to define a single variable that can polymorphically
   * substitute for any RevertExpression<T> without explicitly specifying the
   * type.
   */
  struct RevertPolymorph {
    template<typename T>
    operator RevertExpression<T>() const;

    template<typename T>
    operator Expression<T>() const;
  };

  /**
   * Provides a single 'keyword' like variable that can be used to revert
   * a property.
   */
  constexpr auto revert = RevertPolymorph();

  template<typename T>
  Evaluator<T> make_evaluator(
      RevertExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct RevertEvaluator {
      const Stylist* m_stylist;
      std::type_index m_type;

      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        return m_stylist->revert<Type>(m_type)(frame);
      }
    };
    return RevertEvaluator{&stylist, stylist.m_evaluated_property};
  }

  template<typename T>
  RevertPolymorph::operator RevertExpression<T>() const {
    return RevertExpression<T>();
  }

  template<typename T>
  RevertPolymorph::operator Expression<T>() const {
    return Expression<T>(RevertExpression<T>());
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::RevertExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::RevertExpression<T>& expression) const {
      return 1;
    }
  };
}

#endif
