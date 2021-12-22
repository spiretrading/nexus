#ifndef SPIRE_REFERENCE_VALUE_MODEL_BOX_HPP
#define SPIRE_REFERENCE_VALUE_MODEL_BOX_HPP
#include <functional>
#include "Spire/Spire/ReferenceValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Wraps a type-erased ReferenceValueModel. */
  class ReferenceValueModelBox {
    public:

      /** Signals a change to the value. */
      using UpdateSignal = Signal<void ()>;

      /**
       * Constructs a ReferenceValueModelBox.
       * @param model The model to box.
       */
      template<typename T>
      explicit ReferenceValueModelBox(
        std::shared_ptr<ReferenceValueModel<T>> model);

      /** Returns a reference to the model. */
      const std::shared_ptr<void>& get_model() const;

      /** Signals an update to the underlying model. */
      void signal_update();

      /** Connects a slot to the update signal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      std::shared_ptr<void> m_model;
      std::function<void ()> m_signal_update;
      std::function<boost::signals2::connection (
        const UpdateSignal::slot_type&)> m_connect_update_signal;
  };

  template<typename T>
  ReferenceValueModelBox::ReferenceValueModelBox(
    std::shared_ptr<ReferenceValueModel<T>> model)
    : m_model(std::move(model)),
      m_signal_update([=] {
        static_cast<ReferenceValueModel<T>*>(m_model.get())->signal_update();
      }),
      m_connect_update_signal([=] (const auto& slot) {
        return static_cast<ReferenceValueModel<T>*>(
          m_model.get())->connect_update_signal([=] (const auto&) { slot(); });
      }) {}
}

#endif
