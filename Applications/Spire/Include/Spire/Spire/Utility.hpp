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
}

#endif
