#ifndef SPIRE_POSITION_KEY_BINDINGS_MODEL_HPP
#define SPIRE_POSITION_KEY_BINDINGS_MODEL_HPP
#include <array>
#include "Spire/Ui/KeyInputBox.hpp"

namespace Spire {

  /** Stores key bindings for position operations. */
  class PositionKeyBindingsModel {
    public:

      /** Enumerates the position operations. */
      enum class Operation {

        /** Flattens the selected positions. */
        FLATTEN,

        /** Flattens all open positions. */
        FLATTEN_ALL
      };

      /**
       * Constructs bindings with Ctrl+F for Flatten and Ctrl+Shift+F for
       * Flatten All.
       */
      PositionKeyBindingsModel();

      /** Returns the key binding associated with a position operation. */
      std::shared_ptr<KeySequenceValueModel>
        get_binding(Operation operation) const;

      /** Returns the position operation associated with a key sequence. */
      boost::optional<Operation>
        find_operation(const QKeySequence& sequence) const;

    private:
      static constexpr auto OPERATION_COUNT = 2;
      std::array<std::shared_ptr<KeySequenceValueModel>, OPERATION_COUNT>
        m_bindings;
      std::array<boost::signals2::scoped_connection, OPERATION_COUNT>
        m_connections;

      PositionKeyBindingsModel(const PositionKeyBindingsModel&) = delete;
      PositionKeyBindingsModel& operator =(
        const PositionKeyBindingsModel&) = delete;
      void on_update(Operation operation, const QKeySequence& sequence);
  };
}

#endif
