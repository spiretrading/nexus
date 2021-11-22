#ifndef SPIRE_STYLES_STYLIST_HPP
#define SPIRE_STYLES_STYLIST_HPP
#include <chrono>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/EvaluatedBlock.hpp"
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/StyleSheet.hpp"

namespace Spire::Styles {

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
       * Signals that a Selector was matched or unmatched.
       * @param is_match <code>true</code> iff the associated Selector matches.
       */
      using MatchSignal = Signal<void (bool is_match)>;

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

      /** Connects a slot to the MatchSignal. */
      boost::signals2::connection connect_match_signal(
        const Selector& selector, const MatchSignal::slot_type& slot) const;

    private:
      struct StyleEventFilter;
      struct SelectorHash {
        std::size_t operator ()(const Selector& selector) const;
      };
      struct RuleEntry {
        Rule m_rule;
        int m_priority;
        std::unordered_set<const Stylist*> m_selection;
        SelectConnection m_connection;
      };
      struct Source {
        Stylist* m_source;
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
      QWidget* m_widget;
      boost::optional<PseudoElement> m_pseudo_element;
      StyleSheet m_style;
      std::vector<Source> m_sources;
      std::vector<std::unique_ptr<RuleEntry>> m_rules;
      boost::optional<EvaluatedBlock> m_evaluated_block;
      mutable boost::optional<Block> m_computed_block;
      std::vector<Stylist*> m_proxies;
      std::vector<Stylist*> m_principals;
      std::unordered_set<Selector, SelectorHash> m_matches;
      mutable std::unordered_map<Selector, MatchSignal, SelectorHash>
        m_match_signals;
      std::unordered_map<
        std::type_index, std::unique_ptr<BaseEvaluatorEntry>> m_evaluators;
      std::type_index m_evaluated_property;
      std::chrono::time_point<std::chrono::steady_clock> m_last_frame;
      QMetaObject::Connection m_animation_connection;
      Visibility m_visibility;
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
      void unapply(Stylist& source, const RuleEntry& rule);
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
