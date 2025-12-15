#ifndef SPIRE_CANCEL_KEY_BINDINGS_MODEL_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_MODEL_HPP
#include <array>
#include <unordered_map>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <QKeySequence>
#include <QValidator>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/HashQtTypes.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
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

      /** The number of the cancel operations. */
      static constexpr auto OPERATION_COUNT = 13;

      /** Constructs an empty model. */
      CancelKeyBindingsModel();

      /**
       * Returns the key binding associated with a cancel operation.
       * An empty key sequence indicates an unbound operation.
       * If a cancel operation is bound to an existing key sequence then the
       * existing key sequence is reset.
       */
      std::shared_ptr<KeySequenceValueModel>
        get_binding(Operation operation) const;

      /** Returns the cancel operation associated with a key sequence. */
      boost::optional<Operation>
        find_operation(const QKeySequence& sequence) const;

    private:
      friend struct Beam::Shuttle<CancelKeyBindingsModel>;
      std::array<std::shared_ptr<KeySequenceValueModel>, OPERATION_COUNT>
        m_bindings;
      std::unordered_map<QKeySequence, Operation> m_bindings_map;
      std::array<QKeySequence, OPERATION_COUNT> m_previous_bindings;

      CancelKeyBindingsModel(const CancelKeyBindingsModel&) = delete;
      CancelKeyBindingsModel& operator =(const CancelKeyBindingsModel&) =
        delete;
      QValidator::State on_validate(
        Operation operation, const QKeySequence& sequence);
      void on_update(Operation operation, const QKeySequence& sequence);
  };

  /**
   * Executes a cancel operation on a list of order tasks.
   * @param operation The cancel operation to execute.
   * @param tasks The list of tasks to execute the cancel operation on.
   */
  void execute(CancelKeyBindingsModel::Operation operation,
    Beam::Out<std::vector<std::shared_ptr<Task>>> tasks);

  /**
   * Executes a cancel operation on a list of order entries.
   * @param operation The cancel operation to execute.
   * @param entries The list of order entries to execute the cancel operation
   *        on.
   */
  void execute(CancelKeyBindingsModel::Operation operation,
    Nexus::OrderExecutionClient& client,
    Beam::Out<std::vector<OrderLogModel::OrderEntry>> entries);
}

namespace Beam {
  template<>
  struct Shuttle<Spire::CancelKeyBindingsModel> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::CancelKeyBindingsModel& value,
        unsigned int version) const {
      auto size = static_cast<int>(value.m_bindings.size());
      shuttle.start_sequence("bindings", size);
      for(auto& binding : value.m_bindings) {
        shuttle.shuttle(*binding);
      }
      shuttle.end_sequence();
      shuttle.shuttle("bindings_map", value.m_bindings_map);
      shuttle.shuttle("previous_bindings", value.m_previous_bindings);
    }
  };
}

#endif
