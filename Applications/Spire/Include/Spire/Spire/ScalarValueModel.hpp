#ifndef SPIRE_SCALAR_VALUE_MODEL_HPP
#define SPIRE_SCALAR_VALUE_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct scalar_type {
    using type = T;
  };

  template<typename T>
  struct scalar_type<boost::optional<T>> {
    using type = typename boost::optional<T>::value_type;
  };

  template<typename T>
  using scalar_type_t = typename scalar_type<T>::type;
}

  /**
   * Interface that extends a ValueModel for scalar values.
   * @param <T> The type of value being modeled.
   */
  template<typename T>
  class ScalarValueModel : public ValueModel<T> {
    public:

      /** The type of value being modeled. */
      using Type = typename ValueModel<T>::Type;

      /** The type of scalar value being modeled. */
      using Scalar = Details::scalar_type_t<T>;

      /**
       * Signals a change to the value.
       * @param value The updated value.
       */
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /** Returns the minimum scalar value or none if there is no minimum. */
      virtual boost::optional<Scalar> get_minimum() const;

      /** Returns the maximum scalar value or none if there is no maximum. */
      virtual boost::optional<Scalar> get_maximum() const;

      /** Returns the smallest value by which a value can be increased. */
      virtual boost::optional<Scalar> get_increment() const;
  };

  template<typename T>
  boost::optional<typename ScalarValueModel<T>::Scalar>
      ScalarValueModel<T>::get_minimum() const {
    if constexpr(std::numeric_limits<Scalar>::is_specialized &&
        std::numeric_limits<Scalar>::is_integer) {
      return std::numeric_limits<Scalar>::lowest();
    }
    return boost::none;
  }

  template<typename T>
  boost::optional<typename ScalarValueModel<T>::Scalar>
      ScalarValueModel<T>::get_maximum() const {
    if constexpr(std::numeric_limits<Scalar>::is_specialized &&
        std::numeric_limits<Scalar>::is_integer) {
      return std::numeric_limits<Scalar>::max();
    }
    return boost::none;
  }

  template<typename T>
  boost::optional<typename ScalarValueModel<T>::Scalar>
      ScalarValueModel<T>::get_increment() const {
    return boost::none;
  }
}

#endif
