#include "Spire/Styles/Stylist.hpp"
#include <deque>
#include <boost/functional/hash.hpp>
#include <QApplication>
#include <QTimer>
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/Selectors.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto FRAME_DURATION = time_duration(seconds(1)) / 30;

  QTimer& get_animation_timer() {
    static auto timer = [] {
      auto timer = std::make_unique<QTimer>();
      timer->setInterval(static_cast<int>(FRAME_DURATION.total_milliseconds()));
      timer->start();
      return timer;
    }();
    return *timer;
  }

  auto& get_stylists() {
    static auto stylists = std::unordered_map<QWidget*, Stylist*>();
    return stylists;
  }

  struct PseudoStylistHash {
    std::size_t operator ()(
        const std::pair<QWidget*, PseudoElement>& element) const {
      auto seed = std::size_t(0);
      hash_combine(seed, std::hash<QWidget*>()(element.first));
      hash_combine(seed, std::hash<PseudoElement>()(element.second));
      return seed;
    }
  };

  std::unordered_map<std::pair<QWidget*, PseudoElement>, Stylist*,
    PseudoStylistHash> pseudo_stylists;

  bool contains(QWidget& container, QWidget& widget) {
    if(&container == &widget) {
      return true;
    }
    auto container_rect =
      QRect(container.mapToGlobal(QPoint(0, 0)), container.frameSize());
    auto widget_rect =
      QRect(widget.mapToGlobal(QPoint(0, 0)), widget.frameSize());
    return container_rect.contains(widget_rect, true);
  }

  std::shared_ptr<StyleSheet> load_styles(StyleSheet styles) {
    static auto cache =
      std::unordered_map<StyleSheet, std::shared_ptr<StyleSheet>>();
    auto i = cache.find(styles);
    if(i == cache.end()) {
      auto cached_styles = std::make_shared<StyleSheet>(std::move(styles));
      cache.emplace_hint(i, *cached_styles, cached_styles);
      return cached_styles;
    }
    return i->second;
  }
}

struct Stylist::StyleEventFilter : QObject {
  Stylist* m_stylist;

  StyleEventFilter(Stylist& stylist)
      : QObject(stylist.m_widget),
        m_stylist(&stylist) {
    auto& widget = *stylist.m_widget;
    if(widget.isActiveWindow()) {
      m_stylist->match(Active());
    }
    widget.installEventFilter(this);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::WindowActivate) {
      m_stylist->match(Active());
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_stylist->unmatch(Active());
    } else if(event->type() == QEvent::Show) {
      if(m_stylist->m_widget->isActiveWindow()) {
        m_stylist->match(Active());
      } else {
        m_stylist->unmatch(Active());
      }
    }
    return QObject::eventFilter(watched, event);
  }
};

Stylist::~Stylist() {
  while(!m_matches.empty()) {
    auto selector = *m_matches.begin();
    unmatch(selector);
  }
  m_delete_signal();
  get_animation_timer().disconnect(m_animation_connection);
  for(auto& rule : m_rules) {
    auto selection = std::move(rule->m_selection);
    if(!selection.empty()) {
      on_selection_update(*rule, {}, std::move(selection));
    }
  }
  while(!m_sources.empty()) {
    auto& source = m_sources.back();
    if(source.m_source != this) {
      source.m_source->on_selection_update(
        const_cast<RuleEntry&>(*source.m_rule), {}, {this});
    } else {
      m_sources.pop_back();
    }
  }
  while(!m_proxies.empty()) {
    remove_proxy(*m_proxies.front()->m_widget);
  }
  while(!m_principals.empty()) {
    m_principals.front()->remove_proxy(*m_widget);
  }
  while(!m_links.empty()) {
    std::erase(m_links.back()->m_backlinks, this);
    m_links.pop_back();
  }
  while(!m_backlinks.empty()) {
    std::erase(m_backlinks.back()->m_links, this);
    m_backlinks.pop_back();
  }
}

QWidget& Stylist::get_widget() const {
  return *m_widget;
}

const optional<PseudoElement>& Stylist::get_pseudo_element() const {
  return m_pseudo_element;
}

const StyleSheet& Stylist::get_style() const {
  return *m_style;
}

void Stylist::set_style(StyleSheet style) {
  auto rules = std::exchange(m_rules, {});
  for(auto& rule : rules) {
    auto selection = std::move(rule->m_selection);
    if(!selection.empty()) {
      on_selection_update(*rule, {}, std::move(selection));
    }
  }
  m_style = load_styles(std::move(style));
  apply(*m_style);
}

bool Stylist::is_match(const Selector& selector) const {
  return m_matches.find(selector) != m_matches.end();
}

const Block& Stylist::get_computed_block() const {
  if(m_computed_block) {
    return *m_computed_block;
  }
  m_computed_block.emplace();
  for(auto& source : m_sources) {
    merge(*m_computed_block, source.m_rule->m_block);
  }
  for(auto principal : m_principals) {
    merge(*m_computed_block, principal->get_computed_block());
  }
  return *m_computed_block;
}

const EvaluatedBlock& Stylist::get_evaluated_block() const {
  return *m_evaluated_block;
}

const std::vector<Stylist*>& Stylist::get_proxies() const {
  return m_proxies;
}

const std::vector<Stylist*>& Stylist::get_principals() const {
  return m_principals;
}

void Stylist::add_proxy(QWidget& widget) {
  auto& stylist = find_stylist(widget);
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &stylist);
  if(i == m_proxies.end()) {
    m_proxies.push_back(&stylist);
    stylist.m_principals.push_back(this);
    stylist.apply(*m_style);
  }
}

void Stylist::remove_proxy(QWidget& widget) {
  auto& stylist = find_stylist(widget);
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &stylist);
  if(i == m_proxies.end()) {
    return;
  }
  stylist.m_principals.erase(
    std::find(stylist.m_principals.begin(), stylist.m_principals.end(), this));
  m_proxies.erase(i);
  stylist.apply_proxies();
}

std::vector<const Stylist*> Stylist::get_links() const {
  return std::vector<const Stylist*>(m_links.begin(), m_links.end());
}

const std::vector<Stylist*>& Stylist::get_links() {
  return m_links;
}

std::vector<const Stylist*> Stylist::get_backlinks() const {
  return std::vector<const Stylist*>(m_backlinks.begin(), m_backlinks.end());
}

const std::vector<Stylist*>& Stylist::get_backlinks() {
  return m_backlinks;
}

void Stylist::link(Stylist& target) {
  m_links.push_back(&target);
  target.m_backlinks.push_back(this);
  m_link_signal(target);
  target.m_backlink_signal(*this);
}

void Stylist::match(const Selector& selector) {
  if(m_matches.insert(selector).second) {
    auto signal = m_match_signals.find(selector);
    if(signal != m_match_signals.end()) {
      signal->second(true);
    }
  }
}

void Stylist::unmatch(const Selector& selector) {
  if(m_matches.erase(selector) != 0) {
    auto signal = m_match_signals.find(selector);
    if(signal != m_match_signals.end()) {
      signal->second(false);
    }
  }
}

connection Stylist::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

connection Stylist::connect_link_signal(
    const LinkSignal::slot_type& slot) const {
  return m_link_signal.connect(slot);
}

connection Stylist::connect_backlink_signal(
    const BacklinkSignal::slot_type& slot) const {
  return m_backlink_signal.connect(slot);
}

connection Stylist::connect_match_signal(
    const Selector& selector, const MatchSignal::slot_type& slot) const {
  return m_match_signals[selector].connect(slot);
}

connection Stylist::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

Stylist::Stylist(QWidget& widget, optional<PseudoElement> pseudo_element)
    : m_widget(&widget),
      m_pseudo_element(std::move(pseudo_element)),
      m_style(load_styles(StyleSheet())),
      m_has_visibility(false),
      m_evaluated_block(in_place_init),
      m_evaluated_property(typeid(void)) {
  if(!m_pseudo_element) {
    m_style_event_filter = std::make_unique<StyleEventFilter>(*this);
  }
}

template<typename F>
void Stylist::for_each_principal(F&& f) {
  auto principals = std::deque<Stylist*>();
  principals.push_back(this);
  while(!principals.empty()) {
    auto principal = principals.front();
    principals.pop_front();
    principals.insert(principals.end(), principal->m_principals.begin(),
      principal->m_principals.end());
    std::forward<F>(f)(*principal);
  }
}

template<typename F>
void Stylist::for_each_principal(F&& f) const {
  const_cast<Stylist*>(this)->for_each_principal(std::forward<F>(f));
}

template<typename F>
void Stylist::for_each_proxy(F&& f) {
  auto proxies = std::deque<Stylist*>();
  proxies.push_back(this);
  while(!proxies.empty()) {
    auto proxy = proxies.front();
    proxies.pop_front();
    proxies.insert(
      proxies.end(), proxy->m_proxies.begin(), proxy->m_proxies.end());
    std::forward<F>(f)(*proxy);
  }
}

template<typename F>
void Stylist::for_each_proxy(F&& f) const {
  const_cast<Stylist*>(this)->for_each_proxy(std::forward<F>(f));
}

void Stylist::apply(const StyleSheet& style) {
  static auto priority = 0;
  for(auto& rule : style.get_rules()) {
    auto entry = std::make_unique<RuleEntry>();
    entry->m_priority = priority;
    ++priority;
    entry->m_block = rule.get_block();
    entry->m_connection = select(rule.get_selector(), *this,
      std::bind_front(&Stylist::on_selection_update, this, std::ref(*entry)));
    m_rules.push_back(std::move(entry));
  }
}

void Stylist::apply(Stylist& source, const RuleEntry& rule) {
  auto level = 0;
  auto base = this;
  auto target = &source;
  auto increment = 1;
  while(base != target) {
    if(auto parent = find_parent(*base)) {
      base = parent;
      level += increment;
    } else if(increment == 1) {
      level = 0;
      base = &source;
      target = this;
      increment = -1;
    } else {
      level = 0;
      break;
    }
  }
  auto j = std::lower_bound(m_sources.begin(), m_sources.end(), rule,
    [&] (const auto& left, const auto& right) {
      return std::tie(left.m_level, left.m_rule->m_priority) <
        std::tie(level, right.m_priority);
    });
  m_sources.insert(j, {&source, level, &rule});
}

void Stylist::unapply(const RuleEntry& rule) {
  std::erase_if(m_sources, [&] (const auto& entry) {
    return entry.m_rule == &rule;
  });
}

void Stylist::apply() {
  m_computed_block = none;
  m_evaluated_block.emplace();
  auto& block = get_computed_block();
  if(!m_evaluators.empty()) {
    for(auto i = m_evaluators.begin(); i != m_evaluators.end();) {
      auto property = find(block, i->first);
      if(!property || *property != i->second->m_property) {
        i = m_evaluators.erase(i);
      } else {
        ++i;
      }
    }
    if(m_evaluators.empty()) {
      get_animation_timer().disconnect(m_animation_connection);
    }
  }
  m_style_signal();
  if(m_pseudo_element) {
    return;
  }
  if(auto visibility = Spire::Styles::find<Visibility>(block)) {
    m_has_visibility = true;
    evaluate(*visibility, [=] (auto visibility) {
      if(visibility == Visibility::VISIBLE) {
        m_widget->show();
      } else {
        auto size = m_widget->sizePolicy();
        size.setRetainSizeWhenHidden(visibility == Visibility::INVISIBLE);
        m_widget->setSizePolicy(size);
        m_widget->hide();
      }
    });
  } else if(m_has_visibility && !m_widget->isVisible()) {
    m_widget->show();
  }
}

void Stylist::apply_proxies() {
  apply();
  for(auto proxy : m_proxies) {
    proxy->apply_proxies();
  }
}

optional<Property> Stylist::find_reverted_property(std::type_index type) const {
  auto property = optional<Property>();
  auto reverted_property = optional<Property>();
  for_each_principal([&] (auto& principal) {
    for(auto& source : principal.m_sources) {
      if(auto p = find(source.m_rule->m_block, type)) {
        reverted_property = std::move(property);
        property.emplace(*p);
      }
    }
  });
  return reverted_property;
}

void Stylist::connect_animation() {
  m_animation_connection = QObject::connect(
    &get_animation_timer(), &QTimer::timeout, [=] { on_animation(); });
  m_last_frame = std::chrono::steady_clock::now();
}

void Stylist::on_animation() {
  for(auto& evaluator_pair : m_evaluators) {
    auto delta = time_duration(
      microseconds(std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - m_last_frame).count()));
    auto& evaluator = *evaluator_pair.second;
    evaluator.m_next_frame -= delta;
    if(evaluator.m_next_frame <= seconds(0)) {
      evaluator.animate();
      evaluator.m_elapsed += std::max(delta, evaluator.m_next_frame);
    }
  }
  m_last_frame = std::chrono::steady_clock::now();
}

void Stylist::on_selection_update(
    RuleEntry& rule, std::unordered_set<const Stylist*>&& additions,
    std::unordered_set<const Stylist*>&& removals) {
  auto changed_stylists = std::unordered_set<Stylist*>();
  for(auto removal : removals) {
    rule.m_selection.erase(removal);
    auto& stylist = const_cast<Stylist&>(*removal);
    stylist.unapply(rule);
    changed_stylists.insert(&stylist);
  }
  for(auto addition : additions) {
    rule.m_selection.insert(addition);
    auto& stylist = const_cast<Stylist&>(*addition);
    stylist.apply(*this, rule);
    changed_stylists.insert(&stylist);
  }
  for(auto stylist : changed_stylists) {
    stylist->apply_proxies();
  }
}

const Stylist& Spire::Styles::find_stylist(const QWidget& widget) {
  return find_stylist(const_cast<QWidget&>(widget));
}

const Stylist* Spire::Styles::find_stylist(
    const QWidget& widget, const PseudoElement& pseudo_element) {
  return find_stylist(const_cast<QWidget&>(widget), pseudo_element);
}

Stylist& Spire::Styles::find_stylist(QWidget& widget) {
  auto& stylists = get_stylists();
  auto stylist = stylists.find(&widget);
  if(stylist == stylists.end()) {
    auto entry = new Stylist(widget, none);
    stylist = stylists.insert(std::pair(&widget, entry)).first;
    QObject::connect(&widget, &QObject::destroyed,
      [=, &widget, &stylists] (auto) {
        entry->m_style_event_filter = nullptr;
        delete entry;
        stylists.erase(&widget);
      });
  }
  return *stylist->second;
}

Stylist* Spire::Styles::find_stylist(QWidget& widget,
    const PseudoElement& pseudo_element) {
  auto stylist = pseudo_stylists.find(std::pair(&widget, pseudo_element));
  if(stylist != pseudo_stylists.end()) {
    return &*stylist->second;
  }
  return nullptr;
}

const Stylist* Spire::Styles::find_parent(const Stylist& stylist) {
  return find_parent(const_cast<Stylist&>(stylist));
}

Stylist* Spire::Styles::find_parent(Stylist& stylist) {
  auto parent = stylist.get_widget().parentWidget();
  while(parent != nullptr && &find_stylist(*parent) == &stylist) {
    parent = parent->parentWidget();
  }
  if(parent) {
    return &find_stylist(*parent);
  }
  return nullptr;
}

const StyleSheet& Spire::Styles::get_style(const QWidget& widget) {
  return find_stylist(widget).get_style();
}

const StyleSheet& Spire::Styles::get_style(
    const QWidget& widget, const PseudoElement& pseudo_element) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    return stylist->get_style();
  }
  static auto EMPTY = StyleSheet();
  return EMPTY;
}

void Spire::Styles::set_style(QWidget& widget, StyleSheet style) {
  find_stylist(widget).set_style(std::move(style));
}

void Spire::Styles::set_style(
    QWidget& widget, const PseudoElement& pseudo_element, StyleSheet style) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    stylist->set_style(std::move(style));
  }
}

const Block& Spire::Styles::get_computed_block(QWidget& widget) {
  return find_stylist(widget).get_computed_block();
}

const Block& Spire::Styles::get_computed_block(
    QWidget& widget, const PseudoElement& pseudo_element) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    return stylist->get_computed_block();
  } else {
    static const auto block = Block();
    return block;
  }
}

const EvaluatedBlock&
    Spire::Styles::get_evaluated_block(const QWidget& widget) {
  return find_stylist(widget).get_evaluated_block();
}

const EvaluatedBlock& Spire::Styles::get_evaluated_block(
    QWidget& widget, const PseudoElement& pseudo_element) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    return stylist->get_evaluated_block();
  } else {
    static const auto block = EvaluatedBlock();
    return block;
  }
}

Stylist::BaseEvaluatorEntry::BaseEvaluatorEntry(Property property)
  : m_property(std::move(property)) {}

std::vector<PseudoElement> Spire::Styles::get_pseudo_elements(
    const QWidget& source) {
  auto pseudo_elements = std::vector<PseudoElement>();
  for(auto& pseudo_stylist : pseudo_stylists) {
    if(pseudo_stylist.first.first == &source) {
      pseudo_elements.push_back(pseudo_stylist.first.second);
    }
  }
  return pseudo_elements;
}

void Spire::Styles::add_pseudo_element(
    QWidget& source, const PseudoElement& pseudo_element) {
  auto stylist = pseudo_stylists.find(std::pair(&source, pseudo_element));
  if(stylist != pseudo_stylists.end()) {
    return;
  }
  auto entry = new Stylist(source, pseudo_element);
  stylist = pseudo_stylists.insert(
    std::pair(std::pair(&source, pseudo_element), entry)).first;
  auto& source_stylist = find_stylist(source);
  source_stylist.set_style(source_stylist.get_style());
  QObject::connect(&source, &QObject::destroyed, [=, &source] (QObject*) {
    entry->m_style_event_filter = nullptr;
    delete entry;
    pseudo_stylists.erase(std::pair(&source, pseudo_element));
  });
}

void Spire::Styles::forward_style(QWidget& source, QWidget& destination) {
  auto& stylists = get_stylists();
  assert(!stylists.contains(&source));
  stylists.insert(std::pair(&source, &find_stylist(destination)));
  QObject::connect(&source, &QObject::destroyed,
    [=, &source, &stylists] (QObject*) {
      stylists.erase(&source);
    });
}

void Spire::Styles::proxy_style(QWidget& source, QWidget& destination) {
  find_stylist(source).add_proxy(destination);
}

void Spire::Styles::link(QWidget& root, QWidget& target) {
  find_stylist(root).link(find_stylist(target));
}

bool Spire::Styles::is_match(QWidget& widget, const Selector& selector) {
  return find_stylist(widget).is_match(selector);
}

void Spire::Styles::match(QWidget& widget, const Selector& selector) {
  find_stylist(widget).match(selector);
}

void Spire::Styles::unmatch(QWidget& widget, const Selector& selector) {
  find_stylist(widget).unmatch(selector);
}

connection Spire::Styles::connect_style_signal(const QWidget& widget,
    const Stylist::StyleSignal::slot_type& slot) {
  return find_stylist(widget).connect_style_signal(slot);
}

connection Spire::Styles::connect_style_signal(const QWidget& widget,
    const PseudoElement& pseudo_element,
    const Stylist::StyleSignal::slot_type& slot) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    auto& primary_stylist = find_stylist(widget);
    return stylist->connect_style_signal([=, &primary_stylist] {
      if(primary_stylist.m_widget) {
        slot();
      }
    });
  }
  return {};
}
