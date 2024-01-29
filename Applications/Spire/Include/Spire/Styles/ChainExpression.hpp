#ifndef SPIRE_STYLES_CHAIN_EXPRESSION_HPP
#define SPIRE_STYLES_CHAIN_EXPRESSION_HPP
#include <functional>
#include <memory>
#include <utility>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Chains two Expressions together, evaluating to the first until the first
   * expression terminates, and then evaluating to the second.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class ChainExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a ChainExpression.
       * @param first The first Expression to evaluate to.
       * @param second The Expression to evaluate to after <i>first</i>
       *        terminates.
       */
      ChainExpression(Expression<Type> first, Expression<Type> second);

      /** Returns the first Expression to evaluate to. */
      const Expression<Type>& get_first() const;

      /** Returns the second Expression to evaluate to. */
      const Expression<Type>& get_second() const;

      bool operator ==(const ChainExpression& expression) const = default;

    private:
      Expression<Type> m_first;
      Expression<Type> m_second;
  };

  template<typename T, typename U>
  ChainExpression(T&&, U&&) -> ChainExpression<expression_type_t<T>>;

  /**
   * Chains a series of expressions together to form a single animation.
   * @param expression The series of expressions to chain.
   * @return A ChainExpression sequencing all sub-expressions together.
   */
  template<typename T>
  decltype(auto) chain(T&& expression) {
    return std::forward<T>(expression);
  }

  template<typename T, typename... U>
  auto chain(T&& head, U&&... expression) {
    return ChainExpression(
      std::forward<T>(head), chain(std::forward<U>(expression)...));
  }

  template<typename T>
  auto make_evaluator(ChainExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct ChainEvaluator {
      Evaluator<Type> m_first;
      Evaluator<Type> m_second;
      boost::posix_time::time_duration m_offset;

      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        if(m_offset != boost::posix_time::time_duration() &&
            frame >= m_offset) {
          return m_second(frame - m_offset);
        }
        auto evaluation = m_first(frame);
        if(evaluation.m_next_frame == boost::posix_time::pos_infin) {
          m_offset = frame;
          evaluation.m_next_frame = boost::posix_time::seconds(0);
        }
        return evaluation;
      }
    };
    return ChainEvaluator{make_evaluator(expression.get_first(), stylist),
      make_evaluator(expression.get_second(), stylist)};
  }

  template<typename T>
  ChainExpression<T>::ChainExpression(
    Expression<Type> first, Expression<Type> second)
    : m_first(std::move(first)),
      m_second(std::move(second)) {}

  template<typename T>
  const Expression<typename ChainExpression<T>::Type>&
      ChainExpression<T>::get_first() const {
    return m_first;
  }

  template<typename T>
  const Expression<typename ChainExpression<T>::Type>&
      ChainExpression<T>::get_second() const {
    return m_second;
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::ChainExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::ChainExpression<T>& expression) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_first()));
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_second()));
      return seed;
    }
  };
}

#endif
