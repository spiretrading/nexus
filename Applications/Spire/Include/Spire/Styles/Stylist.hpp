#ifndef SPIRE_STYLES_STYLIST_HPP
#define SPIRE_STYLES_STYLIST_HPP
#include <QWidget>
#include <unordered_set>
#include <vector>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/AndSelector.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/ChildSelector.hpp"
#include "Spire/Styles/DescendantSelector.hpp"
#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/NotSelector.hpp"
#include "Spire/Styles/OrSelector.hpp"
#include "Spire/Styles/ParentSelector.hpp"
#include "Spire/Styles/PropertyMatchSelector.hpp"
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/SiblingSelector.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Styles/VoidSelector.hpp"

namespace Spire::Styles {

  /** Selects the widget that is or belongs to the active window. */
  using Active = StateSelector<void, struct ActiveSelectorTag>;

  /** Selects the disabled widget. */
  using Disabled = StateSelector<void, struct DisabledSelectorTag>;

  /** Selects the hovered widget. */
  using Hover = StateSelector<void, struct HoverSelectorTag>;

  /** Selects the focused widget. */
  using Focus = StateSelector<void, struct FocusSelectorTag>;

  /** Specifies whether an element is visible. */
  enum class VisibilityOption {

    /** The element is visible. */
    VISIBLE,

    /** The element is invisible. */
    INVISIBLE,

    /** The element is treated as if it has a width and height of 0. */
    NONE
  };

  /** Sets the display mode. */
  using Visibility = BasicProperty<VisibilityOption, struct VisibilityTag>;

  /** Keeps track of a widget's styling. */
  class Stylist {
    public:

      /**
       * Signals a change to the styling, typically used to indicate that a
       * widget's style should be re-applied.
       */
      using StyleSignal = Signal<void ()>;

      /**
       * Constructs a Stylist for a widget with an empty StyleSheet.
       * @param widget The widget to style.
       */
      explicit Stylist(QWidget& parent);

      /**
       * Constructs a Stylist.
       * @param style The initial StyleSheet.
       * @param widget The widget to style.
       */
      Stylist(StyleSheet style, QWidget& widget);

      ~Stylist();

      /** Returns the style. */
      const StyleSheet& get_style() const;

      /** Sets the style and update's the QWidget. */
      void set_style(const StyleSheet& style);

      /** Returns <code>true</code> iff a Selector matches. */
      bool is_match(const Selector& selector) const;

      /** Returns a Block containing this widget's computed style. */
      Block compute_style() const;

      /**
       * Specifies that all styles applied to this widget are also applied to
       * another widget.
       * @param widget The widget to proxy styles to.
       */
      void add_proxy(QWidget& widget);

      /**
       * Stops proxying styles from this widget to another.
       * @param widget The widget to stop proxying styles to.
       */
      void remove_proxy(QWidget& widget);

      /**
       * Indicates this widget matches a Selector.
       * @param selector The selector to match.
       */
      void match(const Selector& selector);

      /**
       * Indicates this widget no longer matches a Selector.
       * @param selector The disabled selector.
       */
      void unmatch(const Selector& selector);

      /** Connects a slot to the StyleSignal. */
      boost::signals2::connection connect_style_signal(
        const StyleSignal::slot_type& slot) const;

    private:
      struct StyleEventFilter;
      struct SelectorHash {
        std::size_t operator ()(const Selector& selector) const;
      };
      struct SelectorEquality {
        bool operator ()(const Selector& left, const Selector& right) const;
      };
      struct BlockEntry {
        const QWidget* m_source;
        int m_priority;
        Block m_block;
      };
      using EnableSignal = Signal<void ()>;
      mutable StyleSignal m_style_signal;
      mutable EnableSignal m_enable_signal;
      QWidget* m_widget;
      StyleSheet m_style;
      VisibilityOption m_visibility;
      std::vector<QWidget*> m_principals;
      std::vector<QWidget*> m_proxies;
      std::unordered_set<Selector, SelectorHash, SelectorEquality>
        m_matching_selectors;
      std::unordered_set<QWidget*> m_dependents;
      std::vector<boost::signals2::scoped_connection> m_enable_connections;
      std::unordered_map<const QWidget*, std::shared_ptr<BlockEntry>>
        m_source_to_block;
      std::vector<std::shared_ptr<BlockEntry>> m_blocks;

      Stylist(const Stylist&) = delete;
      Stylist& operator =(const Stylist&) = delete;
      void apply(const QWidget& source, Block block);
      void apply_rules();
      void apply_style();
      void apply_proxy_styles();
      boost::signals2::connection connect_enable_signal(
        const EnableSignal::slot_type& slot) const;
      void on_enable();
  };

  /** Returns the Stylist associated with a widget. */
  const Stylist& find_stylist(const QWidget& widget);

  /** Returns the Stylist associated with a widget. */
  Stylist& find_stylist(QWidget& widget);

  /** Returns a QWidget's styling. */
  const StyleSheet& get_style(const QWidget& widget);

  /** Sets the styling of a QWidget. */
  void set_style(QWidget& widget, const StyleSheet& style);

  /**
   * Specifies that a QWidget will proxy its style to another QWidget.
   * @param principal The QWidget forwarding its style.
   * @param destination The QWidget receiving the style.
   */
  void proxy_style(QWidget& source, QWidget& destination);
}

#endif
