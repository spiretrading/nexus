#ifndef SPIRE_ANY_VALUE_MODEL_HPP
#define SPIRE_ANY_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel by wrapping an existing ValueModel with a type-safe
   * generic interface.
   */
  class AnyValueModel : public ValueModel<AnyRef> {
    public:
      using Type = ValueModel<AnyRef>::Type;

      using UpdateSignal = ValueModel<AnyRef>::UpdateSignal;

      /**
       * Constructs an AnyValueModel over an existing ValueModel.
       * @param source The ValueModel to wrap.
       */
      template<typename T>
      explicit AnyValueModel(std::shared_ptr<ValueModel<T>> source);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;
  };

  template<typename T>
  AnyValueModel::AnyValueModel(std::shared_ptr<ValueModel<T>> source) {}
}

#endif
