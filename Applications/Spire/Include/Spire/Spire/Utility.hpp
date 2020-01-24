#ifndef SPIRE_UTILITY_HPP
#define SPIRE_UTILITY_HPP

namespace Spire {

  //! Returns the minimum of the given parameters. Returns the first given
  //! parameter if they are equal.
  /*!
    \param first The first value to compare.
    \param second The second value to compare.
  */
  template<typename T>
  T min(const T& first, const T& second) {
    if(second < first) {
      return second;
    }
    return first;
  }

  //! Returns the maximum of the given parameters. Returns the first given
  //! parameter if they are equal.
  /*!
    \param first The first value to compare.
    \param second The second value to compare.
  */
  template<typename T>
  T max(const T& first, const T& second) {
    if(first < second) {
      return second;
    }
    return first;
  }

  //! Calculates a value from a second interval as a relative equivalent of a
  //! first interval, where the intervals are both closed.
  /*
    \param value The value to map.
    \param a The minimum value of the first interval.
    \param b The maximum value of the first interval.
    \param c The minimum value of the second interval.
    \param d The maximum value of the second interval.
    \return The value mapped from the first interval to the second interval.
  */
  template<typename T, typename U>
  U map_to(T value, T a, T b, U c, U d) {
    return static_cast<U>((value - a) / (b - a) * (d - c) + c);
  }

  //! Calculates a value from a second interval as a relative equivalent of a
  //! first interval, where the intervals are both closed. Prevents truncation
  //! of integers while mapping the value.
  /*
    \param value The value to map.
    \param a The minimum value of the first interval.
    \param b The maximum value of the first interval.
    \param c The minimum value of the second interval.
    \param d The maximum value of the second interval.
    \return The value mapped from the first interval to the second interval.
  */
  template<typename U>
  U map_to(int value, int a, int b, U c, U d) {
    return map_to(static_cast<double>(value), static_cast<double>(a),
      static_cast<double>(b), c, d);
  }

  //! Calculates a value from a second interval as a relative equivalent of a
  //! first interval, where the intervals are both closed. Prevents truncation
  //! of integers while mapping the value.
  /*
    \param value The value to map.
    \param a The minimum value of the first interval.
    \param b The maximum value of the first interval.
    \param c The minimum value of the second interval.
    \param d The maximum value of the second interval.
    \return The value mapped from the first interval to the second interval.
  */
  template<typename T>
  int map_to(T value, T a, T b, int c, int d) {
    return static_cast<int>(
      map_to(value, a, b, static_cast<double>(c), static_cast<double>(d)));
  }
}

#endif
