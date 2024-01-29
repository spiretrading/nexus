#ifndef SPIRE_SELECT_CONNECTION_HPP
#define SPIRE_SELECT_CONNECTION_HPP
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

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
       * @param args The arguments representing the state of the connection.
       */
      template<typename T, typename... U>
      explicit SelectConnection(T&& arg, U&&... args);

      SelectConnection(const SelectConnection&) = delete;

      SelectConnection(SelectConnection&&) = default;

      SelectConnection& operator =(const SelectConnection&) = delete;

      SelectConnection& operator =(SelectConnection&&) = default;

      /** Returns <code>true</code> iff there is a connection. */
      bool is_connected() const;

    private:
      std::shared_ptr<void> m_state;
  };

  template<typename T, typename... U>
  SelectConnection::SelectConnection(T&& arg, U&&... args)
    : m_state(std::make_shared<
        std::tuple<std::remove_reference_t<T>, std::remove_reference_t<U>...>>(
          std::forward<T>(arg), std::forward<U>(args)...)) {}
}

#endif
