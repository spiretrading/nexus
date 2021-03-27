#ifndef SPIRE_SCALAR_VALUE_MODEL_HPP
#define SPIRE_SCALAR_VALUE_MODEL_HPP
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {

  /**
   * Interface that extends a ValueModel for scalar values.
   * @param <T> The type of value being modeled.
   */
  template<typename T>
  class ScalarValueModel : public ValueModel<T> {
    public:

      /** The type of value being modeled. */
      using Type = typename ValueModel<T>::Type;

      /**
       * Signals a change to the current value.
       * @param current The current value.
       */
      using CurrentSignal = typename ValueModel<T>::CurrentSignal;

      /** Returns the minimum scalar value or none if there is no minimum. */
      virtual boost::optional<Type> get_minimum() const;

      /** Returns the maximum scalar value or none if there is no maximum. */
      virtual boost::optional<Type> get_maximum() const;

      /** Returns the smallest value by which a value can be increased. */
      virtual Type get_increment() const;
  };

  template<typename T>
  boost::optional<typename ScalarValueModel<T>::Type>
      ScalarValueModel<T>::get_minimum() const {
    if constexpr(std::numeric_limits<Type>::is_specialized) {
      return std::numeric_limits<Type>::lowest();
    }
    return boost::none;
  }

  template<typename T>
  boost::optional<typename ScalarValueModel<T>::Type>
      ScalarValueModel<T>::get_maximum() const {
    if constexpr(std::numeric_limits<Type>::is_specialized) {
      return std::numeric_limits<Type>::max();
    }
    return boost::none;
  }

  template<typename T>
  typename ScalarValueModel<T>::Type
      ScalarValueModel<T>::get_increment() const {
    return std::numeric_limits<Type>::max() / std::numeric_limits<Type>::max();
  }
}

#endif
