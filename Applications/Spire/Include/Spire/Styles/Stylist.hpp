#ifndef SPIRE_STYLES_STYLIST_HPP
#define SPIRE_STYLES_STYLIST_HPP
#include <QWidget>
#include <unordered_set>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/AndSelector.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/ChildSelector.hpp"
#include "Spire/Styles/DescendantSelector.hpp"
#include "Spire/Styles/DisambiguateSelector.hpp"
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

      ~Stylist();

      /** Returns the QWidget being styled. */
      QWidget& get_widget();

      /** Returns the PseudoElement represented. */
      const boost::optional<PseudoElement>& get_pseudo_element() const;

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
       * Directs this Stylist to match a Selector.
       * @param selector The selector to match.
       */
      void match(const Selector& selector);

      /**
       * Directs this Stylist to no longer match a Selector.
       * @param selector The selector to no longer match.
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
      struct BlockEntry {
        Stylist* m_source;
        int m_priority;
        Block m_block;
      };
      using EnableSignal = Signal<void ()>;
      friend Stylist& find_stylist(QWidget& widget);
      friend void add_pseudo_element(QWidget& source,
        const PseudoElement& pseudo_element);
      mutable StyleSignal m_style_signal;
      mutable EnableSignal m_enable_signal;
      QWidget* m_widget;
      boost::optional<PseudoElement> m_pseudo_element;
      StyleSheet m_style;
      VisibilityOption m_visibility;
      std::vector<Stylist*> m_principals;
      std::vector<Stylist*> m_proxies;
      std::unordered_set<Selector, SelectorHash> m_matching_selectors;
      std::unordered_set<Stylist*> m_dependents;
      std::vector<boost::signals2::scoped_connection> m_enable_connections;
      std::unordered_map<const Stylist*, std::shared_ptr<BlockEntry>>
        m_source_to_block;
      std::vector<std::shared_ptr<BlockEntry>> m_blocks;

      Stylist(QWidget& parent, boost::optional<PseudoElement> pseudo_element);
      Stylist(const Stylist&) = delete;
      Stylist& operator =(const Stylist&) = delete;
      void remove_dependent(Stylist& dependent);
      void apply(Stylist& source, Block block);
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

  /** Finds the Stylist associated with a widget's pseudo-element. */
  const Stylist* find_stylist(
    const QWidget& widget, const PseudoElement& pseudo_element);

  /** Returns the Stylist associated with a widget. */
  Stylist& find_stylist(QWidget& widget);

  /** Finds the Stylist associated with a widget's pseudo-element. */
  Stylist* find_stylist(QWidget& widget, const PseudoElement& pseudo_element);

  /** Returns a QWidget's styling. */
  const StyleSheet& get_style(const QWidget& widget);

  /** Sets the styling of a QWidget. */
  void set_style(QWidget& widget, const StyleSheet& style);

  /** Returns a Block containing a widget's computed style. */
  Block compute_style(QWidget& widget);

  /**
   * Returns a Block containing the computed style of a widget's pseudoelement.
   */
  Block compute_style(QWidget& widget, const PseudoElement& pseudo_element);

  /** Returns all of a QWidget's PseudoElements. */
  std::vector<PseudoElement> get_pseudo_elements(const QWidget& source);

  /** Associates a PseudoElement with a QWidget. */
  void add_pseudo_element(QWidget& source, const PseudoElement& pseudo_element);

  /**
   * Specifies that a QWidget will proxy its style to another QWidget.
   * @param principal The QWidget forwarding its style.
   * @param destination The QWidget receiving the style.
   */
  void proxy_style(QWidget& source, QWidget& destination);

  /**
   * Indicates a widget no longer matches a Selector.
   * @param widget The widget to match.
   * @param selector The selector to match.
   */
  void match(QWidget& widget, const Selector& selector);

  /**
   * Indicates a widget no longer matches a Selector.
   * @param widget The widget to unmatch.
   * @param selector The selector to no longer match.
   */
  void unmatch(QWidget& widget, const Selector& selector);

  /** Connects a slot to a QWidget's StyleSignal. */
  boost::signals2::connection connect_style_signal(
    const QWidget& widget, const Stylist::StyleSignal::slot_type& slot);

  /** Connects a slot to a QWidget's StyleSignal. */
  boost::signals2::connection connect_style_signal(
    const QWidget& widget, const PseudoElement& pseudo_element,
    const Stylist::StyleSignal::slot_type& slot);
}

#endif
