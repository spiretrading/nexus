#ifndef SPIRE_FIELD_MODEL_HPP
#define SPIRE_FIELD_MODEL_HPP
#include <memory>
#include "Spire/Spire/FieldPointer.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
  template<typename T>
  class FieldModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      template<typename U>
      const std::shared_ptr<ValueModel<typename
        FieldPointer::split_pointer_to_member<U>::field>>& get(U field) const;

    protected:
      virtual void* get(const FieldPointer& member) const = 0;
  };

  template<typename T>
  template<typename U>
  const std::shared_ptr<ValueModel<typename
      FieldPointer::split_pointer_to_member<U>::field>>& FieldModel<T>::get(
        U field) const {
    return *static_cast<std::shared_ptr<ValueModel<
      typename FieldPointer::split_pointer_to_member<U>::field>>*>(
        get(FieldPointer(field)));
  }
}

#endif
