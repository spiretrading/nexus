#ifndef SPIRE_TASK_HPP
#define SPIRE_TASK_HPP
#include <cstdint>
#include <ostream>
#include <string>
#include <Aspen/Aspen.hpp>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/Queues/SnapshotPublisher.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {
namespace Details {
  BEAM_ENUM(TaskState,

    //! The Task is initializing, no sub-Tasks may be executed.
    INITIALIZING,

    //! Task is active, sub-Tasks may be executed.
    ACTIVE,

    //! Task is pending cancel, no new sub-Tasks may be executed.
    PENDING_CANCEL,

    //! Task is canceled.
    CANCELED,

    //! Task encountered an error.
    FAILED,

    //! The Task has expired.
    EXPIRED,

    //! Task has completed.
    COMPLETE);
}

  /** Executes and manages an asynchronous operation and sub-Tasks. */
  class Task {
    public:

      /** Enumerates Task states. */
      using State = Details::TaskState;

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
         * Constructs a StateEntry with an empty message.
         * @param state The Task's State.
         */
        explicit StateEntry(State::Type state);

        /**
         * Constructs a StateEntry.
         * @param state The Task's State.
         * @param message A message describing the change in State.
         */
        StateEntry(State state, const std::string& message);
      };

      using Type = StateEntry;

      /**
       * Constructs a Task.
       * @param reactor The reactor to execute as part of this task.
       */
      explicit Task(Aspen::Box<void> reactor);

      Task(Task&&) = default;

      /** Returns a unique id for this Task. */
      std::int32_t GetId() const;

      /** Cancels this Task. */
      void Cancel();

      /** Returns the Publisher indicating the State of this Task. */
      const Beam::Publisher<StateEntry>& GetPublisher() const;

      Aspen::State commit(int sequence) noexcept;

      const StateEntry& eval() const noexcept;

      Task& operator =(Task&&) = default;

    private:
      Aspen::Box<void> m_reactor;
      std::int32_t m_id;
      std::unique_ptr<Beam::SequencePublisher<StateEntry>> m_publisher;

      Task(const Task&) = delete;
      Task& operator =(const Task&) = delete;
  };

  /** Streams a string representation of a Task's State. */
  std::ostream& operator <<(std::ostream& o, Task::State state);

  /**
   * Tests if a Task's State represents a terminal state.
   * A terminal state is one of CANCELED, FAILED, EXPIRED, or COMPLETE.
   */
  bool IsTerminal(Task::State state);
}

namespace Beam {
  template<>
  struct EnumeratorCount<Spire::Task::State> :
    std::integral_constant<int, 7> {};
}

#endif
