#ifndef SPIRE_SELECT_CONNECTION_HPP
#define SPIRE_SELECT_CONNECTION_HPP
#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

namespace Spire::Styles {
  class Stylist;

  /**
   * The type of callable used to signal a change in the selected Stylists.
   * @param additions The set of Stylists added to the selection.
   * @param removals The set of Stylists removed from the selection.
   */
  using SelectionUpdateSignal = std::function<void (
    std::unordered_set<const Stylist*>&& additions,
    std::unordered_set<const Stylist*>&& removals)>;

  /** Stores a scoped connection to a select operation. */
  class SelectConnection {
    public:

      /** Constructs a stateless connection. */
      SelectConnection() = default;

      /**
       * Constructs a stateful connection.
       * @param executor The object managing the connection and selection
       *        updates.
       */
      template<typename T>
      explicit SelectConnection(std::unique_ptr<T> executor);

      SelectConnection(const SelectConnection&) = delete;
      SelectConnection(SelectConnection&& connection);

      /** Returns <code>true</code> iff there is a connection. */
      bool is_connected() const;

      /** Disconnects this connection. */
      void disconnect();

      SelectConnection& operator =(const SelectConnection&) = delete;
      SelectConnection& operator =(SelectConnection&& connection);

    private:
      struct BaseExecutor {
        virtual ~BaseExecutor() = default;

        virtual bool is_connected() const = 0;
      };
      template<typename T>
      struct Executor final : BaseExecutor {
        std::unique_ptr<T> m_executor;

        explicit Executor(std::unique_ptr<T> executor);

        bool is_connected() const override;
      };
      std::unique_ptr<BaseExecutor> m_executor;
  };

  template<typename T>
  SelectConnection::SelectConnection(std::unique_ptr<T> executor)
    : m_executor(std::make_unique<Executor<T>>(std::move(executor))) {}

  template<typename T>
  SelectConnection::Executor<T>::Executor(std::unique_ptr<T> executor)
    : m_executor(std::move(executor)) {}

  template<typename T>
  bool SelectConnection::Executor<T>::is_connected() const {
    return m_executor->is_connected();
  }
}

#endif
