#ifndef SPIRE_KEY_BINDINGS_MODEL_HPP
#define SPIRE_KEY_BINDINGS_MODEL_HPP
#include <memory>
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Stores a user's key bindings for submitting and cancelling tasks. */
  class KeyBindingsModel {
    public:

      /** Constructs an empty KeyBindingsModel. */
      KeyBindingsModel();

      /** Returns the list of OrderTaskArguments. */
      const std::shared_ptr<OrderTaskArgumentsListModel>&
        get_order_task_arguments() const;

      /** Returns the cancel key bindings. */
      const std::shared_ptr<CancelKeyBindingsModel>&
        get_cancel_key_bindings() const;

      /** Returns the interactions key bindings. */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings() const;

    private:
      std::shared_ptr<OrderTaskArgumentsListModel> m_order_task_arguments;
      std::shared_ptr<CancelKeyBindingsModel> m_cancel_key_bindings;
      std::shared_ptr<InteractionsKeyBindingsModel> m_interactions_key_bindings;

      KeyBindingsModel(const KeyBindingsModel&) = delete;
      KeyBindingsModel& operator =(const KeyBindingsModel&) = delete;
  };
}

#endif
