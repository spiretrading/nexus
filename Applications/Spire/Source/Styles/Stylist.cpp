#include "Spire/Styles/Stylist.hpp"
#include <deque>
#include <QApplication>
#include <QFocusEvent>
#include <QTimer>
#include <boost/functional/hash.hpp>
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

  std::unordered_map<std::pair<QWidget*, PseudoElement>, Stylist*,
    boost::hash<std::pair<QWidget*, PseudoElement>>> pseudo_stylists;

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
}

struct Stylist::StyleEventFilter : QObject {
  Stylist* m_stylist;

  StyleEventFilter(Stylist& stylist)
      : QObject(stylist.m_widget),
        m_stylist(&stylist) {
    auto& widget = *stylist.m_widget;
    if(widget.hasFocus()) {
      m_stylist->match(Focus());
    }
    if(!widget.isEnabled()) {
      m_stylist->match(Disabled());
    }
    if(widget.isActiveWindow()) {
      m_stylist->match(Active());
    }
    connect(qApp,
      &QApplication::focusChanged, this, &StyleEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn) {
      m_stylist->match(Focus());
      auto& focus_event = static_cast<const QFocusEvent&>(*event);
      if(focus_event.reason() == Qt::TabFocus ||
          focus_event.reason() == Qt::BacktabFocusReason) {
        m_stylist->match(FocusVisible());
      }
    } else if(event->type() == QEvent::FocusOut) {
      m_stylist->unmatch(Focus());
      m_stylist->unmatch(FocusVisible());
    } else if(event->type() == QEvent::Enter) {
      if(m_stylist->m_widget->isEnabled()) {
        m_stylist->match(Hover());
      }
    } else if(event->type() == QEvent::Leave) {
      poll_mouse();
    } else if(event->type() == QEvent::EnabledChange) {
      if(!m_stylist->m_widget->isEnabled()) {
        m_stylist->match(Disabled());
      } else {
        m_stylist->unmatch(Disabled());
      }
    } else if(event->type() == QEvent::WindowActivate) {
      m_stylist->match(Active());
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_stylist->unmatch(Active());
    } else if(event->type() == QEvent::Show) {
      if(m_stylist->m_widget->isActiveWindow()) {
        m_stylist->match(Active());
      } else {
        m_stylist->unmatch(Active());
      }
    } else if(event->type() == QEvent::ParentChange) {
      m_stylist->set_style(m_stylist->get_style());
    }
    return QObject::eventFilter(watched, event);
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    auto proxy = m_stylist->m_widget->focusProxy();
    while(proxy) {
      if(proxy == now) {
        m_stylist->match(Focus());
        break;
      } else if(proxy == old) {
        m_stylist->unmatch(Focus());
        break;
      }
      proxy = proxy->focusProxy();
    }
  }

  void poll_mouse() {
    if(!m_stylist->get_widget().rect().contains(
        m_stylist->get_widget().mapFromGlobal(QCursor::pos()))) {
      m_stylist->unmatch(Hover());
    } else {
      auto hovered_widget = qApp->widgetAt(QCursor::pos());
      if(hovered_widget && contains(m_stylist->get_widget(), *hovered_widget)) {
        QTimer::singleShot(50, this, [=] { poll_mouse(); });
      } else {
        m_stylist->unmatch(Hover());
      }
    }
  }
};

std::size_t Stylist::SelectorHash::operator ()(const Selector& selector) const {
  return selector.get_type().hash_code();
}

Stylist::~Stylist() {
  m_widget = nullptr;
  m_evaluators.clear();
  get_animation_timer().disconnect(m_animation_connection);
  for(auto& block : m_source_to_block) {
    block.first->m_dependents.erase(this);
  }
  while(!m_dependents.empty()) {
    remove_dependent(**m_dependents.begin());
  }
  while(!m_proxies.empty()) {
    remove_proxy(*m_proxies.front()->m_widget);
  }
  while(!m_principals.empty()) {
    m_principals.front()->remove_proxy(*m_widget);
  }
}

const QWidget& Stylist::get_widget() const {
  return *m_widget;
}

QWidget& Stylist::get_widget() {
  return *m_widget;
}

const optional<PseudoElement>& Stylist::get_pseudo_element() const {
  return m_pseudo_element;
}

const StyleSheet& Stylist::get_style() const {
  return *m_style;
}

void Stylist::set_style(StyleSheet style) {
  m_style = std::make_shared<StyleSheet>(std::move(style));
  m_enable_connections.clear();
  for(auto widget : build_reach(*m_style, *m_widget)) {
    if(widget != m_widget) {
      m_enable_connections.push_back(
        find_stylist(*widget).connect_enable_signal([=] { on_enable(); }));
    }
  }
  apply_rules();
}

bool Stylist::is_match(const Selector& selector) const {
  return m_matching_selectors.find(selector) != m_matching_selectors.end();
}

const Block& Stylist::get_computed_block() const {
  if(m_computed_block) {
    return *m_computed_block;
  }
  m_computed_block.emplace();
  for(auto& entry : m_blocks) {
    merge(*m_computed_block, entry->m_properties);
  }
  for(auto principal : m_principals) {
    ::merge(*m_computed_block, principal->get_computed_block());
  }
  return *m_computed_block;
}

const EvaluatedBlock& Stylist::get_evaluated_block() const {
  return *m_evaluated_block;
}

void Stylist::add_proxy(QWidget& widget) {
  auto& stylist = find_stylist(widget);
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &stylist);
  if(i == m_proxies.end()) {
    m_proxies.push_back(&stylist);
    stylist.m_principals.push_back(this);
    stylist.apply_proxy_styles();
  }
}

void Stylist::remove_proxy(QWidget& widget) {
  auto& stylist = find_stylist(widget);
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &stylist);
  if(i == m_proxies.end()) {
    return;
  }
  stylist.m_principals.erase(std::find(stylist.m_principals.begin(),
    stylist.m_principals.end(), this));
  m_proxies.erase(i);
  stylist.apply_proxy_styles();
}

void Stylist::match(const Selector& selector) {
  if(m_matching_selectors.insert(selector).second) {
    apply_rules();
    auto match_signal = m_match_signals.find(selector);
    if(match_signal != m_match_signals.end()) {
      match_signal->second(true);
    }
    m_enable_signal();
  }
}

void Stylist::unmatch(const Selector& selector) {
  if(m_matching_selectors.erase(selector) != 0) {
    apply_rules();
    auto match_signal = m_match_signals.find(selector);
    if(match_signal != m_match_signals.end()) {
      match_signal->second(false);
    }
    m_enable_signal();
  }
}

connection Stylist::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

connection Stylist::connect_match_signal(
    const Selector& selector, const MatchSignal::slot_type& slot) const {
  return m_match_signals[selector].connect(slot);
}

Stylist::Stylist(QWidget& widget, boost::optional<PseudoElement> pseudo_element)
    : m_widget(&widget),
      m_pseudo_element(std::move(pseudo_element)),
      m_style(std::make_shared<StyleSheet>()),
      m_visibility(Visibility::VISIBLE),
      m_evaluated_block(in_place_init),
      m_evaluated_property(typeid(void)),
      m_is_handling_enabled_signal(false) {
  if(!m_pseudo_element) {
    m_style_event_filter = std::make_unique<StyleEventFilter>(*this);
    m_widget->installEventFilter(m_style_event_filter.get());
  }
}

void Stylist::merge(
    Block& block, const std::vector<AppliedProperty>& properties) {
  for(auto& property : properties) {
    block.set(property.m_property);
  }
}

void Stylist::merge(std::vector<AppliedProperty>& properties,
    std::shared_ptr<const Rule> rule) {
  for(auto& block_property : rule->get_block()) {
    auto is_found = false;
    for(auto& applied_property : properties) {
      if(applied_property.m_property.get_type() == block_property.get_type()) {
        applied_property.m_property = block_property;
        applied_property.m_rule = rule;
        is_found = true;
        break;
      }
    }
    if(!is_found) {
      properties.push_back({block_property, rule});
    }
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
    std::forward<F>(f)(principal);
  }
}

template<typename F>
void Stylist::for_each_principal(F&& f) const {
  auto principals = std::deque<const Stylist*>();
  principals.push_back(this);
  while(!principals.empty()) {
    auto principal = principals.front();
    principals.pop_front();
    principals.insert(principals.end(), principal->m_principals.begin(),
      principal->m_principals.end());
    std::forward<F>(f)(principal);
  }
}

void Stylist::remove_dependent(Stylist& dependent) {
  dependent.apply(*this, {});
  dependent.m_source_to_block.erase(this);
  dependent.m_blocks.erase(
    std::find_if(dependent.m_blocks.begin(), dependent.m_blocks.end(),
      [&] (const auto& block) {
        return block->m_source == this;
      }));
  m_dependents.erase(&dependent);
}

void Stylist::apply(Stylist& source, std::vector<AppliedProperty> properties) {
  auto i = m_source_to_block.find(&source);
  if(i == m_source_to_block.end()) {
    auto entry = std::make_shared<BlockEntry>();
    entry->m_source = &source;
    m_blocks.push_back(entry);
    i = m_source_to_block.insert(std::pair(&source, std::move(entry))).first;
  }
  auto& block = i->second;
  auto has_update = false;
  if(block->m_properties.size() != properties.size()) {
    block->m_properties = std::move(properties);
    has_update = true;
  } else {
    for(auto& applied_property : properties) {
      auto i = std::find_if(block->m_properties.begin(),
        block->m_properties.end(), [&] (const auto& property) {
          return property.m_property.get_type() ==
            applied_property.m_property.get_type() &&
              property.m_rule == applied_property.m_rule;
        });
      if(i == block->m_properties.end()) {
        block->m_properties = std::move(properties);
        has_update = true;
        break;
      }
    }
  }
  if(has_update) {
    apply_proxy_styles();
  }
}

void Stylist::apply_rules() {
  auto applied_properties =
    std::unordered_map<Stylist*, std::vector<AppliedProperty>>();
  for_each_principal([&] (auto principal) {
    for(auto& base_rule : principal->m_style->get_rules()) {
      auto selection = select(base_rule.get_selector(), *this);
      auto rule = std::shared_ptr<const Rule>(principal->m_style, &base_rule);
      for(auto& selected : selection) {
        merge(applied_properties[selected], rule);
      }
    }
  });
  auto previous_dependents = std::move(m_dependents);
  for(auto& properties : applied_properties) {
    m_dependents.insert(properties.first);
    properties.first->apply(*this, std::move(properties.second));
  }
  for(auto previous_dependent : previous_dependents) {
    if(m_dependents.find(previous_dependent) == m_dependents.end()) {
      remove_dependent(*previous_dependent);
    }
  }
  for(auto proxy : m_proxies) {
    proxy->apply_rules();
  }
}

void Stylist::apply_style() {
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
    evaluate(*visibility, [=] (auto visibility) {
      if(visibility != m_visibility) {
        if(visibility == Visibility::VISIBLE) {
          m_widget->show();
        } else if(visibility == Visibility::NONE) {
          auto size = m_widget->sizePolicy();
          size.setRetainSizeWhenHidden(false);
          m_widget->setSizePolicy(size);
          m_widget->hide();
        } else if(visibility == Visibility::INVISIBLE) {
          auto size = m_widget->sizePolicy();
          size.setRetainSizeWhenHidden(true);
          m_widget->setSizePolicy(size);
          m_widget->hide();
        }
        m_visibility = visibility;
      }
    });
  } else if(m_visibility != Visibility::VISIBLE) {
    m_widget->show();
    m_visibility = Visibility::VISIBLE;
  }
}

void Stylist::apply_proxy_styles() {
  apply_style();
  for(auto proxy : m_proxies) {
    proxy->apply_proxy_styles();
  }
}

optional<Property> Stylist::find_reverted_property(std::type_index type) const {
  auto property = boost::optional<Property>();
  auto reverted_property = boost::optional<Property>();
  auto targets = std::unordered_set<const Stylist*>();
  for_each_principal([&] (auto principal) {
    targets.insert(principal);
  });
  auto contains_one_of = [] (const std::unordered_set<Stylist*>& container,
      const std::unordered_set<const Stylist*>& targets) {
    if(container.size() <= targets.size()) {
      for(auto target : container) {
        if(targets.find(target) != targets.end()) {
          return true;
        }
      }
    } else {
      for(auto target : targets) {
        if(container.find(const_cast<Stylist*>(target)) != container.end()) {
          return true;
        }
      }
    }
    return false;
  };
  auto swap_properties = [&] (const Rule& rule) {
    if(auto update = find(rule.get_block(), type)) {
      if(property) {
        reverted_property.emplace(std::move(*property));
      }
      property = std::move(update);
    }
  };
  for(auto& source : m_blocks) {
    source->m_source->for_each_principal([&] (auto principal) {
      auto sources = std::unordered_set{source->m_source, principal};
      for(auto& rule : principal->m_style->get_rules()) {
        auto selection = select(rule.get_selector(), sources);
        if(contains_one_of(selection, targets)) {
          swap_properties(rule);
        }
      }
    });
  }
  return reverted_property;
}

connection Stylist::connect_enable_signal(
    const EnableSignal::slot_type& slot) const {
  return m_enable_signal.connect(slot);
}

void Stylist::connect_animation() {
  m_animation_connection = QObject::connect(
    &get_animation_timer(), &QTimer::timeout, [=] { on_animation(); });
  m_last_frame = std::chrono::steady_clock::now();
}

void Stylist::on_enable() {
  if(m_is_handling_enabled_signal) {
    return;
  }
  m_is_handling_enabled_signal = true;
  apply_rules();
  m_enable_signal();
  m_is_handling_enabled_signal = false;
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

const Stylist& Spire::Styles::find_stylist(const QWidget& widget) {
  return find_stylist(const_cast<QWidget&>(widget));
}

const Stylist* Spire::Styles::find_stylist(const QWidget& widget,
    const PseudoElement& pseudo_element) {
  return find_stylist(const_cast<QWidget&>(widget), pseudo_element);
}

Stylist& Spire::Styles::find_stylist(QWidget& widget) {
  static auto stylists = std::unordered_map<QWidget*, Stylist*>();
  auto stylist = stylists.find(&widget);
  if(stylist == stylists.end()) {
    auto entry = new Stylist(widget, none);
    stylist = stylists.insert(std::pair(&widget, entry)).first;
    QObject::connect(&widget, &QObject::destroyed, [=, &widget] (QObject*) {
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

const StyleSheet& Spire::Styles::get_style(const QWidget& widget) {
  return find_stylist(widget).get_style();
}

void Spire::Styles::set_style(QWidget& widget, StyleSheet style) {
  find_stylist(widget).set_style(std::move(style));
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

const EvaluatedBlock& Spire::Styles::get_evaluated_block(const QWidget& widget) {
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

void Spire::Styles::add_pseudo_element(QWidget& source,
    const PseudoElement& pseudo_element) {
  auto stylist = pseudo_stylists.find(std::pair(&source, pseudo_element));
  if(stylist != pseudo_stylists.end()) {
    return;
  }
  auto entry = new Stylist(source, pseudo_element);
  stylist = pseudo_stylists.insert(
    std::pair(std::pair(&source, pseudo_element), entry)).first;
  QObject::connect(&source, &QObject::destroyed, [=, &source] (QObject*) {
    entry->m_style_event_filter = nullptr;
    delete entry;
    pseudo_stylists.erase(std::pair(&source, pseudo_element));
  });
}

void Spire::Styles::proxy_style(QWidget& source, QWidget& destination) {
  find_stylist(source).add_proxy(destination);
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
