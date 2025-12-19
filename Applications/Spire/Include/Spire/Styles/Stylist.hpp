#ifndef SPIRE_STYLES_STYLIST_HPP
#define SPIRE_STYLES_STYLIST_HPP
#include <chrono>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/EvaluatedBlock.hpp"
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/StyleSheet.hpp"

namespace Spire::Styles {
  template<typename T> class RevertExpression;

  /** Specifies whether an element is visible. */
  enum class Visibility {

    /** The element is visible. */
    VISIBLE,

    /** The element is invisible. */
    INVISIBLE,

    /** The element is treated as if it has a width and height of 0. */
    NONE
  };

  /** Keeps track of a widget's styling. */
  class Stylist {
    public:

      /**
       * Signals a change to the styling, typically used to indicate that a
       * widget's style should be re-applied.
       */
      using StyleSignal = Signal<void ()>;

      /**
       * Signals a link was added to another stylist.
       * @param stylist The stylist being linked to.
       */
      using LinkSignal = Signal<void (const Stylist& stylist)>;

      /**
       * Signals a backlink was added to another stylist.
       * @param stylist The stylist being backlinked to.
       */
      using BacklinkSignal = Signal<void (const Stylist& stylist)>;

      /**
       * Signals that a Selector was matched or unmatched.
       * @param is_match <code>true</code> iff the associated Selector matches.
       */
      using MatchSignal = Signal<void (bool is_match)>;

      /** Signals that this Stylist is being deleted. */
      using DeleteSignal = Signal<void ()>;

      ~Stylist();

      /** Returns the QWidget being styled. */
      QWidget& get_widget() const;

      /** Returns the PseudoElement represented. */
      const boost::optional<PseudoElement>& get_pseudo_element() const;

      /** Returns the style. */
      const StyleSheet& get_style() const;

      /** Sets the style and update's the QWidget. */
      void set_style(StyleSheet style);

      /** Returns <code>true</code> iff a Selector matches. */
      bool is_match(const Selector& selector) const;

      /** Returns the Block containing all current Properties. */
      const Block& get_computed_block() const;

      /**
       * Returns the EvaluatedBlock containing all current EvaluatedProperties.
       */
      const EvaluatedBlock& get_evaluated_block() const;

      /** Returns the list of Stylists being proxied. */
      const std::vector<Stylist*>& get_proxies() const;

      /** Returns the list of Stylists that proxy into this. */
      const std::vector<Stylist*>& get_principals() const;

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

      /** Returns the list of links from this. */
      std::vector<const Stylist*> get_links() const;

      /** Returns the list of links from this. */
      const std::vector<Stylist*>& get_links();

      /** Returns the list of backlinks from this. */
      std::vector<const Stylist*> get_backlinks() const;

      /** Returns the list of backlinks from this. */
      const std::vector<Stylist*>& get_backlinks();

      /**
       * Adds a link from this stylist to a target stylist, allowing the target
       * to be selected through this along a path.
       * @param target The stylist to link from this.
       */
      void link(Stylist& target);

      /** Returns the set of matched Selectors. */
      const std::unordered_set<Selector>& get_matches() const;

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

      /** Directs this Stylist to no longer match any Selector. */
      void unmatch_all();

      /**
       * Applies a function to the evaluation of a property's expression.
       * The function receiving the evaluation may be called multiple times,
       * especially if the property evaluates to an animation.
       * @param property The property whose expression is to be evaluated.
       * @param receiver The function receiving the evaluation.
       */
      template<typename Property, typename F>
      void evaluate(const Property& property, F&& receiver);

      /** Connects a slot to the StyleSignal. */
      boost::signals2::connection connect_style_signal(
        const StyleSignal::slot_type& slot) const;

      /** Connects a slot to the LinkSignal. */
      boost::signals2::connection connect_link_signal(
        const LinkSignal::slot_type& slot) const;

      /** Connects a slot to the BackLinkSignal. */
      boost::signals2::connection connect_backlink_signal(
        const BacklinkSignal::slot_type& slot) const;

      /** Connects a slot to the MatchSignal. */
      boost::signals2::connection connect_match_signal(
        const Selector& selector, const MatchSignal::slot_type& slot) const;

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    private:
      struct StyleEventFilter;
      struct RuleEntry {
        Block m_block;
        int m_priority;
        std::unordered_set<const Stylist*> m_selection;
        SelectConnection m_connection;
      };
      struct Source {
        Stylist* m_source;
        int m_level;
        const RuleEntry* m_rule;
      };
      struct BaseEvaluatorEntry {
        Property m_property;
        boost::posix_time::time_duration m_elapsed;
        boost::posix_time::time_duration m_next_frame;

        BaseEvaluatorEntry(Property property);
        virtual ~BaseEvaluatorEntry() = default;
        virtual void animate() = 0;
      };
      template<typename T>
      struct EvaluatorEntry : BaseEvaluatorEntry {
        using Type = T;
        Evaluator<Type> m_evaluator;
        std::vector<std::function<void (const Type&)>> m_receivers;

        EvaluatorEntry(Property property, Evaluator<Type> evaluator);
        void animate() override;
      };
      friend Stylist& find_stylist(QWidget& widget);
      friend void add_pseudo_element(
        QWidget& source, const PseudoElement& pseudo_element);
      friend boost::signals2::connection connect_style_signal(
        const QWidget& widget, const PseudoElement& pseudo_element,
        const Stylist::StyleSignal::slot_type& slot);
      template<typename T>
      friend Evaluator<T> make_evaluator(
        RevertExpression<T> expression, const Stylist& stylist);
      mutable StyleSignal m_style_signal;
      mutable LinkSignal m_link_signal;
      mutable BacklinkSignal m_backlink_signal;
      mutable DeleteSignal m_delete_signal;
      QWidget* m_widget;
      boost::optional<PseudoElement> m_pseudo_element;
      std::shared_ptr<StyleSheet> m_style;
      std::vector<Source> m_sources;
      std::vector<std::unique_ptr<RuleEntry>> m_rules;
      boost::optional<EvaluatedBlock> m_evaluated_block;
      mutable boost::optional<Block> m_computed_block;
      std::vector<Stylist*> m_proxies;
      std::vector<Stylist*> m_principals;
      std::unordered_set<Selector> m_matches;
      mutable std::unordered_map<Selector, MatchSignal> m_match_signals;
      std::vector<Stylist*> m_links;
      std::vector<Stylist*> m_backlinks;
      std::unordered_map<
        std::type_index, std::unique_ptr<BaseEvaluatorEntry>> m_evaluators;
      std::type_index m_evaluated_property;
      std::chrono::time_point<std::chrono::steady_clock> m_last_frame;
      QMetaObject::Connection m_animation_connection;
      bool m_has_visibility;
      std::unique_ptr<StyleEventFilter> m_style_event_filter;

      Stylist(QWidget& parent, boost::optional<PseudoElement> pseudo_element);
      Stylist(const Stylist&) = delete;
      Stylist& operator =(const Stylist&) = delete;
      template<typename F>
      void for_each_principal(F&& f);
      template<typename F>
      void for_each_principal(F&& f) const;
      template<typename F>
      void for_each_proxy(F&& f);
      template<typename F>
      void for_each_proxy(F&& f) const;
      void apply(const StyleSheet& style);
      void apply(Stylist& source, const RuleEntry& rule);
      void unapply(const RuleEntry& rule);
      void apply();
      void apply_proxies();
      boost::optional<Property> find_reverted_property(
        std::type_index type) const;
      template<typename T>
      Evaluator<T> revert(std::type_index type) const;
      void connect_animation();
      void on_animation();
      void on_selection_update(RuleEntry& rule,
        std::unordered_set<const Stylist*>&& additions,
        std::unordered_set<const Stylist*>&& removals);
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

  /**
   * Returns a Stylist's parent or <i>nullptr</i> if the Stylist is the root.
   */
  const Stylist* find_parent(const Stylist& stylist);

  /**
   * Returns a Stylist's parent or <i>nullptr</i> if the Stylist is the root.
   */
  Stylist* find_parent(Stylist& stylist);

  /** Returns a QWidget's styling. */
  const StyleSheet& get_style(const QWidget& widget);

  /** Returns a QWidget's styling. */
  const StyleSheet& get_style(
    const QWidget& widget, const PseudoElement& pseudo_element);

  /** Sets the styling of a QWidget. */
  void set_style(QWidget& widget, StyleSheet style);

  /** Sets the styling of a QWidget. */
  void set_style(QWidget& widget, const PseudoElement& pseudo_element,
    StyleSheet style);

  /** Updates a QWidget's style. */
  template<typename F>
  void update_style(QWidget& widget, F&& f) {
    auto style = get_style(widget);
    std::forward<F>(f)(style);
    set_style(widget, std::move(style));
  }

  /** Returns a Block containing a widget's computed style. */
  const Block& get_computed_block(QWidget& widget);

  /**
   * Returns a Block containing the computed style of a widget's pseudoelement.
   */
  const Block& get_computed_block(
    QWidget& widget, const PseudoElement& pseudo_element);

  /** Returns an EvaluatedBlock containing a widget's evaluated style. */
  const EvaluatedBlock& get_evaluated_block(const QWidget& widget);

  /**
   * Returns a Block containing the evaluated style of a widget's pseudoelement.
   */
  const EvaluatedBlock& get_evaluated_block(
    QWidget& widget, const PseudoElement& pseudo_element);

  /** Returns all of a QWidget's PseudoElements. */
  std::vector<PseudoElement> get_pseudo_elements(const QWidget& source);

  /** Associates a PseudoElement with a QWidget. */
  void add_pseudo_element(QWidget& source, const PseudoElement& pseudo_element);

  /**
   * Specifies that a QWidget does not have a style of its own but instead
   * any styling applied to it will get forwarded to another QWidget.
   * @param principal The QWidget forwarding its style.
   * @param destination The QWidget receiving the style.
   */
  void forward_style(QWidget& principal, QWidget& destination);

  /**
   * Specifies that a QWidget will proxy its style to another QWidget.
   * @param principal The QWidget proxying its style.
   * @param destination The QWidget receiving the style.
   */
  void proxy_style(QWidget& principal, QWidget& destination);

  /**
   * Adds a link from a root QWidget to another QWidget, allowing it to be
   * accessed through the root along a path.
   * @param root The root of the link.
   * @param target The QWidget to link from the root.
   */
  void link(QWidget& root, QWidget& target);

  /**
   * Returns <code>true</code> iff a widget matches a selector.
   * @param widget The widget to test.
   * @param selector The selector to check for a match.
   * @return <code>true</code> iff the <i>widget</i> matches the
   *         <i>selector</i>.
   */
  bool is_match(QWidget& widget, const Selector& selector);

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

  /**
   * Unmatches all Selectors.
   * @param widget The widget to unmatch from all selectors.
   */
  void unmatch_all(QWidget& widget);

  /** Connects a slot to a QWidget's StyleSignal. */
  boost::signals2::connection connect_style_signal(
    const QWidget& widget, const Stylist::StyleSignal::slot_type& slot);

  /** Connects a slot to a QWidget's StyleSignal. */
  boost::signals2::connection connect_style_signal(
    const QWidget& widget, const PseudoElement& pseudo_element,
    const Stylist::StyleSignal::slot_type& slot);

  template<typename T>
  Stylist::EvaluatorEntry<T>::EvaluatorEntry(
    Property property, Evaluator<Type> evaluator)
    : BaseEvaluatorEntry(std::move(property)),
      m_evaluator(std::move(evaluator)) {}

  template<typename T>
  void Stylist::EvaluatorEntry<T>::animate() {
    auto evaluation = m_evaluator(m_elapsed);
    for(auto i = m_receivers.begin(); i != m_receivers.end() - 1; ++i) {
      (*i)(evaluation.m_value);
    }
    m_receivers.back()(std::move(evaluation.m_value));
    m_next_frame = evaluation.m_next_frame;
  }

  template<typename Property, typename F>
  void Stylist::evaluate(const Property& property, F&& receiver) {
    m_evaluated_property = typeid(Property);
    auto i = m_evaluators.find(m_evaluated_property);
    if(i == m_evaluators.end()) {
      auto evaluator = make_evaluator(property.get_expression(), *this);
      auto evaluation = evaluator(boost::posix_time::seconds(0));
      if(evaluation.m_next_frame != boost::posix_time::pos_infin) {
        auto entry = std::make_unique<EvaluatorEntry<typename Property::Type>>(
          property, std::move(evaluator));
        entry->m_receivers.push_back(std::forward<F>(receiver));
        auto& receiver = entry->m_receivers.back();
        m_evaluators.emplace(m_evaluated_property, std::move(entry));
        if(m_evaluators.size() == 1) {
          connect_animation();
        }
        m_evaluated_block->set(
          EvaluatedProperty(std::in_place_type<Property>, evaluation.m_value));
        receiver(std::move(evaluation.m_value));
      } else {
        m_evaluated_block->set(
          EvaluatedProperty(std::in_place_type<Property>, evaluation.m_value));
        std::forward<F>(receiver)(std::move(evaluation.m_value));
      }
    } else {
      auto& evaluator = static_cast<EvaluatorEntry<typename Property::Type>&>(
        *i->second);
      evaluator.m_receivers.push_back(std::forward<F>(receiver));
      auto evaluation = evaluator.m_evaluator(evaluator.m_elapsed).m_value;
      m_evaluated_block->set(
        EvaluatedProperty(std::in_place_type<Property>, evaluation));
      evaluator.m_receivers.back()(std::move(evaluation));
    }
  }

  template<typename T>
  Evaluator<T> Stylist::revert(std::type_index type) const {
    auto reverted_property = find_reverted_property(type);
    if(!reverted_property) {
      return [] (auto frame) {
        if constexpr(std::is_same_v<T, QColor>) {
          return Evaluation(QColor(0, 0, 0, 0));
        } else {
          return Evaluation(T());
        }
      };
    }
    return make_evaluator(reverted_property->expression_as<T>(), *this);
  }
}

#endif
