#ifndef SPIRE_FIELD_MODEL_HPP
#define SPIRE_FIELD_MODEL_HPP
#include <memory>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T> struct type_of_field {};

  template<typename T, typename U>
  struct type_of_field<U T::*> {
    using type = U;
  };
}

  template<typename T>
  class FieldModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      template<typename U>
      const std::shared_ptr<ValueModel<
        typename Details::type_of_field<U>::type>>& get(U field) const;

    protected:
      virtual void* get(void* member) const = 0;

      template<typename U>
      static void* lookup(U field) {
        
      }
  };

  template<typename T>
  template<typename U>
  const std::shared_ptr<ValueModel<typename Details::type_of_field<U>::type>>&
      FieldModel<T>::get(U field) const {
    return *static_cast<std::shared_ptr<
      ValueModel<typename Details::type_of_field<U>::type>>*>(get((void*)(field)));
  }
}

#endif
