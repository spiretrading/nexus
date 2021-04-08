#ifndef SPIRE_STYLES_STYLED_WIDGET_HPP
#define SPIRE_STYLES_STYLED_WIDGET_HPP
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

  /** Base class for a QWidget styled according to a StyleSheet. */
  class StyledWidget : public QWidget {
    public:

      /**
       * Constructs a StyledWidget with an empty StyleSheet.
       * @param parent The parent widget.
       * @param flags The initial window flags.
       */
      explicit StyledWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /**
       * Constructs a StyledWidget.
       * @param style The initial StyleSheet.
       * @param parent The parent widget.
       * @param flags The initial window flags.
       */
      explicit StyledWidget(StyleSheet style, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      ~StyledWidget() override;

      /** Returns the style. */
      const StyleSheet& get_style() const;

      /** Sets the style and update's the QWidget. */
      void set_style(const StyleSheet& style);

      /** Returns <code>true</code> iff a Selector is enabled. */
      bool is_enabled(const Selector& selector) const;

      /** Returns a Block containing this widget's computed style. */
      Block compute_style() const;

      /** Returns a Block containing the computed style for a pseudo-element. */
      Block compute_style(const Selector& element) const;

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

    protected:

      /**
       * Indicates a selector is enabled for this widget.
       * @param selector The enabled selector.
       */
      void enable(const Selector& selector);

      /**
       * Indicates a selector is no longer enabled for this widget.
       * @param selector The disabled selector.
       */
      void disable(const Selector& selector);

      /**
       * Applies the computed style to this widget.
       * Called whenever there's a change to the computed style.
       */
      virtual void apply_style();

    private:
      friend class SelectorRegistry;
      struct StyleEventFilter;
      struct SelectorHash {
        std::size_t operator ()(const Selector& selector) const;
      };
      struct SelectorEquality {
        bool operator ()(const Selector& left, const Selector& right) const;
      };
      struct BlockEntry {
        const StyledWidget* m_source;
        int m_priority;
        Block m_block;
      };
      using EnableSignal = Signal<void ()>;
      mutable EnableSignal m_enable_signal;
      StyleSheet m_style;
      VisibilityOption m_visibility;
      std::vector<StyledWidget*> m_principals;
      std::vector<StyledWidget*> m_proxies;
      std::unordered_set<Selector, SelectorHash, SelectorEquality>
        m_enabled_selectors;
      std::unordered_set<StyledWidget*> m_dependents;
      std::vector<boost::signals2::scoped_connection> m_enable_connections;
      std::unordered_map<const StyledWidget*, std::shared_ptr<BlockEntry>>
        m_source_to_block;
      std::vector<std::shared_ptr<BlockEntry>> m_blocks;

      void apply(const StyledWidget& source, Block block);
      void apply_rules();
      void apply_proxy_styles();
      boost::signals2::connection connect_enable_signal(
        const EnableSignal::slot_type& slot) const;
      void on_enable();
  };

  std::vector<QWidget*> select(const Active& selector, QWidget& source);
  std::vector<QWidget*> select(const Disabled& selector, QWidget& source);
  std::vector<QWidget*> select(const Hover& selector, QWidget& source);
  std::vector<QWidget*> select(const Focus& selector, QWidget& source);
}

#endif
