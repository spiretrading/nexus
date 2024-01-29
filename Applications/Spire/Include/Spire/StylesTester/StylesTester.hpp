#ifndef SPIRE_STYLES_TESTER_HPP
#define SPIRE_STYLES_TESTER_HPP
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"

class QWidget;

namespace Spire::Styles::Tests {

  /** Properties provided for testing purposes. */
  using ColorProperty = BasicProperty<QColor, struct ColorPropertyTag>;
  using IntProperty = BasicProperty<int, struct IntTag>;
  using ColorIntProperty = CompositeProperty<ColorProperty, IntProperty>;

  /** Selectors provided for testing purposes. */
  using Foo = StateSelector<void, struct FooSelectorTag>;
  using Bar = StateSelector<void, struct BarSelectorTag>;
  using Qux = StateSelector<void, struct QuxSelectorTag>;

  /** Stores the arguments passed to a SelectionUpdateSignal. */
  struct SelectionUpdate {

    /** The additions passed to the update slot. */
    std::unordered_set<const Stylist*> m_additions;

    /** The removals passed to the update slot. */
    std::unordered_set<const Stylist*> m_removals;
  };

  /**
   * Returns a graph of QWidget's structured as per the below diagram:
   *        A
   *        |
   *        B
   *       / \
   *      C   D
   *     / \ / \
   *    E  F G  H
   */
  std::unordered_map<std::string, QWidget*> make_graph();

  /**
   * Tests that there is an SelectionUpdate containing additions and removals
   * indexed by name from a specified graph.
   * Pops off the update from the update queue.
   * @param updates The queue of updates used to verify the selection.
   * @param graph The graph of QWidgets to test indexed by name.
   * @param additions The expected set of QWidget's added to the selection.
   * @param removals The expected set of QWidget's removed from the selection.
   */
  void require_selection(std::deque<SelectionUpdate>& updates,
    const std::unordered_map<std::string, QWidget*>& graph,
    const std::unordered_set<std::string>& expected_additions,
    const std::unordered_set<std::string>& expected_removals);
}

#endif
