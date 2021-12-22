#ifndef SPIRE_FIELD_MODEL_HPP
#define SPIRE_FIELD_MODEL_HPP
#include <memory>
#include "Spire/Spire/FieldPointer.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel over a value composed of multiple sub-fields.
   * In addition to accessing the value as a whole, each sub-field can be
   * accessed by its own ValueModel.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class FieldModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Returns a ValueModel accessing a sub-field.
       * @param field A pointer to the member variable to access.
       */
      template<typename U>
      const std::shared_ptr<ValueModel<typename
        FieldPointer::split_pointer_to_member<U>::field>>& get(U field) const;

    protected:

      /** Implements a type erased version of the <i>get</i> method. */
      virtual const void* get(const FieldPointer& member) const = 0;
  };

  template<typename T>
  template<typename U>
  const std::shared_ptr<ValueModel<typename
      FieldPointer::split_pointer_to_member<U>::field>>& FieldModel<T>::get(
        U field) const {
    return *static_cast<const std::shared_ptr<ValueModel<
      typename FieldPointer::split_pointer_to_member<U>::field>>*>(
        get(FieldPointer(field)));
  }
}

#endif
