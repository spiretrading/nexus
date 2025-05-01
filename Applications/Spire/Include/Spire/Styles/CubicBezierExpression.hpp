#ifndef SPIRE_STYLES_CUBIC_BEZIER_EXPRESSION_HPP
#define SPIRE_STYLES_CUBIC_BEZIER_EXPRESSION_HPP
#include <memory>
#include <utility>
#include <Beam/Utilities/HashPtime.hpp>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Calculates a parametric cubic-Bezier expression. The calculation fixes the
   * Bezier endpoints from (0, 0) to (1, 1), accepts two control points
   * (x1, y1) and (x2, y2) in that unit square, and then remaps the resulting
   * curve over any start/end values over a given duration.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class CubicBezierExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a cubic-Bezier expression.
       * @param start Expression yielding the value at t = 0.
       * @param end Expression yielding the value at t = duration.
       * @param x1 X-coordinate of first Bezier control point
       *        (must lie in [0,1]).
       * @param y1 Y-coordinate of first Bezier control point.
       * @param x2 X-coordinate of second Bezier control point
       *        (must lie in [0,1]).
       * @param y2 Y-coordinate of second Bezier control point.
       * @param duration Total span of the animation from start to end.
       */
      CubicBezierExpression(Expression<Type> start, Expression<Type> end,
        double x1, double y1, double x2, double y2,
        boost::posix_time::time_duration duration);

      /** Returns the start-value expression. */
      const Expression<Type>& get_start() const;

      /** Returns the end-value expression. */
      const Expression<Type>& get_end() const;

      /** Returns the X coordinate of the first control point. */
      double get_x1() const;

      /** Returns the Y coordinate of the first control point. */
      double get_y1() const;

      /** Returns the X coordinate of the second control point. */
      double get_x2() const;

      /** Returns the Y coordinate of the second control point. */
      double get_y2() const;

      /** Returns the duration of the transition. */
      boost::posix_time::time_duration get_duration() const;

      bool operator==(const CubicBezierExpression& expression) const = default;

    private:
      Expression<Type> m_start;
      Expression<Type> m_end;
      double m_x1;
      double m_y1;
      double m_x2;
      double m_y2;
      boost::posix_time::time_duration m_duration;
  };

  template<typename S, typename U>
  CubicBezierExpression(S&&, U&&, double, double, double, double,
    boost::posix_time::time_duration) ->
      CubicBezierExpression<expression_type_t<S>>;

  /** Calculates the cubic-Bezier curve between two sub-expressions. */
  template<typename S, typename U>
  auto cubic_bezier(S&& start, U&& end, double x1, double y1, double x2,
      double y2, boost::posix_time::time_duration duration) {
    return CubicBezierExpression(
      std::forward<S>(start), std::forward<U>(end), x1, y1, x2, y2, duration);
  }

  template<typename S, typename U>
  auto ease(S&& start, U&& end, boost::posix_time::time_duration duration) {
    return cubic_bezier(std::forward<S>(start), std::forward<U>(end), 0.25, 0.1,
      0.25, 1, duration);
  }

  template<typename T>
  auto make_evaluator(
      CubicBezierExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct CubicBezierEvaluator {
      Evaluator<Type> m_start;
      Evaluator<Type> m_end;
      double m_x1;
      double m_y1;
      double m_x2;
      double m_y2;
      boost::posix_time::time_duration m_duration;

      static double compute_bezier(double a1, double a2, double t) {
        auto u = 1.0 - t;
        return 3.0 * a1 * u * u * t + 3.0 * a2 * u * t * t + t * t * t;
      }

      static double compute_bezier_derivative(double a1, double a2, double t) {
        auto u = 1.0 - t;
        return 3.0 * u * u * a1 +
          6.0 * u * t * (a2 - a1) + 3.0 * t * t * (1.0 - a2);
      }

      static double solve(double x1, double x2, double t) {
        auto u = t;
        for(auto i = 0; i < 4; ++i) {
          auto f  = compute_bezier(x1, x2, u) - t;
          auto df = compute_bezier_derivative(x1, x2, u);
          if(std::abs(df) < 1e-6) {
            break;
          }
          u -= f/df;
          if(u <= 0.0) {
            u = 0.0;
            break;
          } else if(u >= 1.0) {
            u = 1.0;
            break;
          }
        }
        return u;
      }

      Evaluation<Type> operator()(boost::posix_time::time_duration frame) {
        if(frame <= boost::posix_time::seconds(0)) {
          return Evaluation(m_start(boost::posix_time::seconds(0)).m_value,
            boost::posix_time::seconds(0));
        } else if(frame >= m_duration) {
          return Evaluation(
            m_end(m_duration).m_value, boost::posix_time::pos_infin);
        }
        auto t = double(frame.total_milliseconds()) /
          double(m_duration.total_milliseconds());
        auto u = solve(m_x1, m_x2, t);
        auto y = compute_bezier(m_y1, m_y2, u);
        auto s = m_start(frame).m_value;
        auto e = m_end(frame).m_value;
        auto value = Type(s + (e - s) * y);
        auto next_frame = [&] () -> boost::posix_time::time_duration {
          if(frame >= m_duration) {
            return boost::posix_time::pos_infin;
          }
          return boost::posix_time::seconds(0);
        }();
        return Evaluation<T>(std::move(value), next_frame);
      }
    };
    return CubicBezierEvaluator(make_evaluator(expression.get_start(), stylist),
      make_evaluator(expression.get_end(), stylist), expression.get_x1(),
      expression.get_y1(), expression.get_x2(), expression.get_y2(),
      expression.get_duration());
  }

  template<typename T>
  CubicBezierExpression<T>::CubicBezierExpression(Expression<Type> start,
    Expression<Type> end, double x1, double y1, double x2, double y2,
    boost::posix_time::time_duration duration)
    : m_start(std::move(start)),
      m_end(std::move(end)),
      m_x1(x1),
      m_y1(y1),
      m_x2(x2),
      m_y2(y2),
      m_duration(duration) {}

  template<typename T>
  const Expression<typename CubicBezierExpression<T>::Type>&
      CubicBezierExpression<T>::get_start() const {
    return m_start;
  }

  template<typename T>
  const Expression<typename CubicBezierExpression<T>::Type>&
      CubicBezierExpression<T>::get_end() const {
    return m_end;
  }

  template<typename T>
  double CubicBezierExpression<T>::get_x1() const {
    return m_x1;
  }

  template<typename T>
  double CubicBezierExpression<T>::get_y1() const {
    return m_y1;
  }

  template<typename T>
  double CubicBezierExpression<T>::get_x2() const {
    return m_x2;
  }

  template<typename T>
  double CubicBezierExpression<T>::get_y2() const {
    return m_y2;
  }

  template<typename T>
  boost::posix_time::time_duration
      CubicBezierExpression<T>::get_duration() const {
    return m_duration;
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::CubicBezierExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::CubicBezierExpression<T>& expression) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_start()));
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_end()));
      boost::hash_combine(seed, std::hash<double>()(expression.get_x1()));
      boost::hash_combine(seed, std::hash<double>()(expression.get_y1()));
      boost::hash_combine(seed, std::hash<double>()(expression.get_x2()));
      boost::hash_combine(seed, std::hash<double>()(expression.get_y2()));
      boost::hash_combine(seed, std::hash<boost::posix_time::time_duration>()(
        expression.get_duration()));
      return seed;
    }
  };
}

#endif
