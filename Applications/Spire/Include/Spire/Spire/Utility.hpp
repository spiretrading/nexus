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
    if(second > first) {
      return second;
    }
    return first;
  }
}

#endif
