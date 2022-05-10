#ifndef SPIRE_TASK_HPP
#define SPIRE_TASK_HPP
#include <ostream>
#include <string>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Executes and manages an asynchronous operation. */
  class Task {
    public:

      /** Enumerates Task states. */
      enum class State {

        /** The Task is ready to be executed. */
        READY,

        /** The Task is initializing, no sub-Tasks may be executed. */
        INITIALIZING,

        /** Task is active, sub-Tasks may be executed. */
        ACTIVE,

        /** Task is pending cancel, no new sub-Tasks may be executed. */
        PENDING_CANCEL,

        /** Task is canceled. */
        CANCELED,

        /** Task encountered an error. */
        FAILED,

        /** Task has completed. */
        COMPLETE
      };

      /** Records a change in a Task's State. */
      struct StateEntry {

        /** The State of the Task. */
        State m_state;

        /** A message describing the change in State. */
        std::string m_message;

        /** Constructs an uninitialized StateEntry. */
        StateEntry() = default;

        /**
         * Constructs a StateEntry with an empty message.
         * @param state The Task's State.
         */
        explicit StateEntry(State state);

        /**
         * Constructs a StateEntry.
         * @param state The Task's State.
         * @param message A message describing the change in State.
         */
        StateEntry(State state, std::string message);
      };

      /**
       * Constructs a Task in the READY state.
       * @param id The task's unique id.
       */
      explicit Task(int id);

      /** Returns a unique id for this Task. */
      int get_id() const;

      /** Executes this Task. */
      void execute();

      /** Cancels this Task. */
      void cancel();

      /** Returns the Publisher publishing the State of this Task. */
      const Beam::Publisher<StateEntry>& get_publisher() const;

    private:
      int m_id;
      Beam::SequencePublisher<StateEntry> m_publisher;

      Task(const Task&) = delete;
      Task(Task&&) = delete;
      Task& operator =(const Task&) = delete;
      Task& operator =(Task&&) = delete;
  };

  std::ostream& operator <<(std::ostream& out, Task::State state);

  /**
   * Tests if a Task's State represents a terminal state.
   * A terminal state is one of CANCELED, FAILED, or COMPLETE.
   */
  bool is_terminal(Task::State state);
}

#endif
