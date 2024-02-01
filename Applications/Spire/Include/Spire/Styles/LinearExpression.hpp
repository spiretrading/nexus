#ifndef SPIRE_STYLES_LINEAR_EXPRESSION_HPP
#define SPIRE_STYLES_LINEAR_EXPRESSION_HPP
#include <memory>
#include <utility>
#include <Beam/Utilities/HashPtime.hpp>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Calculates a linear transition between two sub-expressions.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class LinearExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a LinearExpression.
       * @param initial The initial value.
       * @param end The end value.
       * @param duration The duration of the transition.
       */
      LinearExpression(Expression<Type> initial, Expression<Type> end,
        boost::posix_time::time_duration duration);

      /** Returns the initial value Expression. */
      const Expression<Type>& get_initial() const;

      /** Returns the end value Expression. */
      const Expression<Type>& get_end() const;

      /** Returns the duration of the transition. */
      boost::posix_time::time_duration get_duration() const;

      bool operator ==(const LinearExpression& expression) const = default;

    private:
      Expression<Type> m_initial;
      Expression<Type> m_end;
      boost::posix_time::time_duration m_duration;
  };

  template<typename T, typename U>
  LinearExpression(T&&, U&&, boost::posix_time::time_duration) ->
    LinearExpression<expression_type_t<T>>;

  /**
   * Calculates a linear transition between two sub-expressions.
   * @param initial The initial value.
   * @param end The end value.
   * @param duration The duration of the transition.
   */
  template<typename T, typename U>
  auto linear(T&& initial, U&& end, boost::posix_time::time_duration duration) {
    return LinearExpression(
      std::forward<T>(initial), std::forward<U>(end), duration);
  }

  template<typename T>
  auto make_evaluator(LinearExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct LinearEvaluator {
      Evaluator<Type> m_initial;
      Evaluator<Type> m_end;
      boost::posix_time::time_duration m_duration;

      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        auto initial = m_initial(frame).m_value;
        auto end = m_end(frame).m_value;
        auto percentage = std::min(
          1.0, frame.ticks() / static_cast<double>(m_duration.ticks()));
        auto transition = [&] (const auto& initial, const auto& end) {
          return initial +
            static_cast<decltype(end)>((end - initial) * percentage);
        };
        auto value = [&] {
          if constexpr(std::is_same_v<Type, QColor>) {
            if(end.alpha() == 0) {
              return QColor(initial.red(), initial.green(), initial.blue(),
                transition(initial.alpha(), end.alpha()));
            }
            return QColor(transition(initial.red(), end.red()),
              transition(initial.green(), end.green()),
              transition(initial.blue(), end.blue()),
              transition(initial.alpha(), end.alpha()));
          } else {
            return transition(initial, end);
          }
        }();
        auto next_frame = [&] () -> boost::posix_time::time_duration {
          if(frame >= m_duration) {
            return boost::posix_time::pos_infin;
          }
          return boost::posix_time::seconds(0);
        }();
        return Evaluation(std::move(value), next_frame);
      }
    };
    return LinearEvaluator{make_evaluator(expression.get_initial(), stylist),
      make_evaluator(expression.get_end(), stylist), expression.get_duration()};
  }

  template<typename T>
  LinearExpression<T>::LinearExpression(Expression<Type> initial,
    Expression<Type> end, boost::posix_time::time_duration duration)
    : m_initial(std::move(initial)),
      m_end(std::move(end)),
      m_duration(duration) {}

  template<typename T>
  const Expression<typename LinearExpression<T>::Type>&
      LinearExpression<T>::get_initial() const {
    return m_initial;
  }

  template<typename T>
  const Expression<typename LinearExpression<T>::Type>&
      LinearExpression<T>::get_end() const {
    return m_end;
  }

  template<typename T>
  boost::posix_time::time_duration LinearExpression<T>::get_duration() const {
    return m_duration;
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::LinearExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::LinearExpression<T>& expression) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_initial()));
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_end()));
      boost::hash_combine(seed, std::hash<boost::posix_time::time_duration>()(
        expression.get_duration()));
      return seed;
    }
  };
}

#endif
