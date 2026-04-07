#ifndef SPIRE_UTILITY_HPP
#define SPIRE_UTILITY_HPP
#include <cmath>
#include <QObject>

namespace Spire {

  /**
   * A deleter which deletes QObjects using deleteLater().
   */
  struct QObjectDeleter {

    /**
     * Deletes the given QObject.
     * @param object The object to delete.
     */
    void operator ()(QObject* object) const noexcept;
  };

  /**
   * A unique pointer which deletes QObjects using deleteLater().
   */
  template<typename T>
  using unique_qt_ptr = std::unique_ptr<T, QObjectDeleter>;

  /**
   * Deletes an object when control returns to the thread's event loop.
   * @param object The object to schedule for deletion.
   */
  template<typename T>
  std::enable_if_t<std::is_base_of_v<QObject, T>> delete_later(T*& object) {
    if(object) {
      object->deleteLater();
      object = nullptr;
    }
  }

  /**
   * Determines if a given widget is an ancestor of another, regardless of which
   * window either widget is in.
   * @param ancestor The potential ancestor.
   * @param descendant The descendant.
   * @returns True iff the given ancestor is an ancestor of the given
   *          descendant.
   */
  bool is_ancestor(const QWidget* ancestor, const QWidget* descendant);

  /**
   * Calculates a value from a second interval as a relative equivalent of a
   * first interval, where the intervals are both closed.
   * @param value The value to map.
   * @param a The minimum value of the first interval.
   * @param b The maximum value of the first interval.
   * @param c The minimum value of the second interval.
   * @param d The maximum value of the second interval.
   * @returns The value mapped from the first interval to the second interval.
   */
  template<typename T, typename U>
  U map_to(T value, T a, T b, U c, U d) {
    if(value - a == a - a) {
      return c;
    }
    return static_cast<U>((value - a) / (b - a) * (d - c) + c);
  }

  /**
   * Calculates a value from a second interval as a relative equivalent of a
   * first interval, where the intervals are both closed. Prevents truncation
   * of integers while mapping the value.
   * @param value The value to map.
   * @param a The minimum value of the first interval.
   * @param b The maximum value of the first interval.
   * @param c The minimum value of the second interval.
   * @param d The maximum value of the second interval.
   * @returns The value mapped from the first interval to the second interval.
   */
  template<typename U>
  U map_to(int value, int a, int b, U c, U d) {
    return map_to(static_cast<double>(value), static_cast<double>(a),
      static_cast<double>(b), c, d);
  }

  /**
   * Calculates a value from a second interval as a relative equivalent of a
   * first interval, where the intervals are both closed. Prevents truncation
   * of integers while mapping the value.
   * @param value The value to map.
   * @param a The minimum value of the first interval.
   * @param b The maximum value of the first interval.
   * @param c The minimum value of the second interval.
   * @param d The maximum value of the second interval.
   * @returns The value mapped from the first interval to the second interval.
   */
  template<typename T>
  int map_to(T value, T a, T b, int c, int d) {
    return static_cast<int>(std::round(
      map_to(value, a, b, static_cast<double>(c), static_cast<double>(d))));
  }
}

#endif
