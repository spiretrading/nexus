#ifndef SPIRE_CANCEL_KEY_BINDINGS_MODEL_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_MODEL_HPP
#include <unordered_map>
#include <QValidator>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/KeyInputBox.hpp"

namespace Spire {

  /** Stores the key bindings for all cancel operations. */
  class CancelKeyBindingsModel {
    public:

      /** Enumerates the cancel operations. */
      enum class Operation {

        /** Cancels the most recent Task. */
        MOST_RECENT,

        /** Cancels the most recent ask Task. */
        MOST_RECENT_ASK,

        /** Cancels the most recent bid Task. */
        MOST_RECENT_BID,

        /** Cancels the oldest Task. */
        OLDEST,

        /** Cancels the oldest ask Task. */
        OLDEST_ASK,

        /** Cancels the oldest bid Task. */
        OLDEST_BID,

        /** Cancels all Tasks. */
        ALL,

        /** Cancels all asks. */
        ALL_ASKS,

        /** Cancels all bids. */
        ALL_BIDS,

        /** Cancels the closest ask by price then time. */
        CLOSEST_ASK,

        /** Cancels the closest bid by price then time. */
        CLOSEST_BID,

        /** Cancels the furthest ask by price then time. */
        FURTHEST_ASK,

        /** Cancels the furthest bid by price then time. */
        FURTHEST_BID
      };

      /** Constructs an empty model. */
      CancelKeyBindingsModel();

      /**
       * Returns the key binding associated with a cancel operation.
       * An empty key sequence indicates an unbound operation.
       * If a cancel operation is bound to an existing key sequence
       * then the existing key sequence is reset.
       */
      std::shared_ptr<KeySequenceValueModel>
        get_binding(Operation operation) const;

      /** Returns the cancel operation associated with a key sequence. */
      boost::optional<Operation>
        find_operation(const QKeySequence& sequence) const;

    private:
      std::unordered_map<Operation, std::shared_ptr<KeySequenceValueModel>>
        m_bindings;

      CancelKeyBindingsModel(const CancelKeyBindingsModel&) = delete;
      CancelKeyBindingsModel& operator =(const CancelKeyBindingsModel&)
        = delete;
      QValidator::State on_validate(Operation operation,
        const QKeySequence& sequence);
  };
}

#endif
