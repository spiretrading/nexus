#include "Spire/Styles/Stylist.hpp"
#include <deque>
#include <QApplication>
#include <QEvent>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/PseudoElement.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::unordered_map<std::pair<QWidget*, PseudoElement>, Stylist*,
    boost::hash<std::pair<QWidget*, PseudoElement>>> pseudo_stylists;
}

struct Stylist::StyleEventFilter : QObject {
  Stylist* m_stylist;

  StyleEventFilter(Stylist& stylist)
      : QObject(stylist.m_widget),
        m_stylist(&stylist) {
    connect(qApp, &QApplication::focusChanged, this,
      &StyleEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn) {
      m_stylist->match(Focus());
    } else if(event->type() == QEvent::FocusOut) {
      m_stylist->unmatch(Focus());
    } else if(event->type() == QEvent::Enter) {
      m_stylist->match(Hover());
    } else if(event->type() == QEvent::Leave) {
      m_stylist->unmatch(Hover());
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
};

std::size_t Stylist::SelectorHash::operator ()(const Selector& selector) const {
  return selector.get_type().hash_code();
}

Stylist::~Stylist() {
  while(!m_dependents.empty()) {
    remove_dependent(**m_dependents.begin());
  }
  for(auto& block : m_source_to_block) {
    block.second->m_source->m_dependents.erase(this);
  }
  while(!m_proxies.empty()) {
    remove_proxy(*m_proxies.front()->m_widget);
  }
  while(!m_principals.empty()) {
    m_principals.front()->remove_proxy(*m_widget);
  }
}

QWidget& Stylist::get_widget() {
  return *m_widget;
}

const optional<PseudoElement>& Stylist::get_pseudo_element() const {
  return m_pseudo_element;
}

const StyleSheet& Stylist::get_style() const {
  return m_style;
}

void Stylist::set_style(const StyleSheet& style) {
  m_style = style;
  m_enable_connections.clear();
  auto reach = build_reach(m_style, *m_widget);
  for(auto widget : reach) {
    if(widget != m_widget) {
      m_enable_connections.push_back(
        find_stylist(*widget).connect_enable_signal([=] { on_enable(); }));
    }
  }
  apply_rules();
}

bool Stylist::is_match(const Selector& selector) const {
  if(m_matching_selectors.find(selector) != m_matching_selectors.end()) {
    return true;
  }
  for(auto proxy : m_proxies) {
    if(proxy->is_match(selector)) {
      return true;
    }
  }
  return false;
}

Block Stylist::compute_style() const {
  auto block = Block();
  for(auto& entry : m_blocks) {
    merge(block, entry->m_block);
  }
  for(auto principal : m_principals) {
    merge(block, principal->compute_style());
  }
  return block;
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
    auto principals = std::deque<Stylist*>();
    principals.push_back(this);
    while(!principals.empty()) {
      auto principal = principals.front();
      principals.pop_front();
      principals.insert(principals.end(), principal->m_principals.begin(),
        principal->m_principals.end());
      principal->m_enable_signal();
      principal->apply_rules();
    }
  }
}

void Stylist::unmatch(const Selector& selector) {
  if(m_matching_selectors.erase(selector) != 0) {
    auto principals = std::deque<Stylist*>();
    principals.push_back(this);
    while(!principals.empty()) {
      auto principal = principals.front();
      principals.pop_front();
      principals.insert(principals.end(), principal->m_principals.begin(),
        principal->m_principals.end());
      principal->m_enable_signal();
      principal->apply_rules();
    }
  }
}

connection Stylist::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

Stylist::Stylist(QWidget& widget, boost::optional<PseudoElement> pseudo_element)
    : m_widget(&widget),
      m_pseudo_element(std::move(pseudo_element)),
      m_visibility(VisibilityOption::VISIBLE) {
  if(!m_pseudo_element) {
    m_widget->installEventFilter(new StyleEventFilter(*this));
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

void Stylist::apply(Stylist& source, Block block) {
  auto i = m_source_to_block.find(&source);
  if(i == m_source_to_block.end()) {
    auto entry = std::make_shared<BlockEntry>();
    entry->m_source = &source;
    entry->m_priority = 0;
    m_blocks.push_back(entry);
    i = m_source_to_block.insert(i, std::pair(&source, std::move(entry)));
  }
  i->second->m_block = std::move(block);
  apply_proxy_styles();
}

void Stylist::apply_rules() {
  auto blocks = std::unordered_map<Stylist*, Block>();
  auto principals = std::deque<Stylist*>();
  principals.push_back(this);
  while(!principals.empty()) {
    auto principal = principals.front();
    principals.pop_front();
    principals.insert(principals.end(), principal->m_principals.begin(),
      principal->m_principals.end());
    for(auto& rule : principal->m_style.get_rules()) {
      auto selection = select(rule.get_selector(), *this);
      for(auto& selected : selection) {
        merge(blocks[selected], rule.get_block());
      }
    }
  }
  auto previous_dependents = std::move(m_dependents);
  for(auto& block : blocks) {
    m_dependents.insert(block.first);
    block.first->apply(*this, std::move(block.second));
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
  m_style_signal();
  if(m_pseudo_element) {
    return;
  }
  auto style = compute_style();
  auto visibility_option = [&] {
    if(auto visibility = Spire::Styles::find<Visibility>(style)) {
      return visibility->get_expression().as<VisibilityOption>();
    }
    return VisibilityOption::VISIBLE;
  }();
  if(visibility_option != m_visibility) {
    if(visibility_option == VisibilityOption::VISIBLE) {
      m_widget->show();
    } else if(visibility_option == VisibilityOption::NONE) {
      auto size = m_widget->sizePolicy();
      size.setRetainSizeWhenHidden(false);
      m_widget->setSizePolicy(size);
      m_widget->hide();
    } else if(visibility_option == VisibilityOption::INVISIBLE) {
      auto size = m_widget->sizePolicy();
      size.setRetainSizeWhenHidden(true);
      m_widget->setSizePolicy(size);
      m_widget->hide();
    }
    m_visibility = visibility_option;
  }
}

void Stylist::apply_proxy_styles() {
  apply_style();
  for(auto proxy : m_proxies) {
    proxy->apply_proxy_styles();
  }
}

connection Stylist::connect_enable_signal(
    const EnableSignal::slot_type& slot) const {
  return m_enable_signal.connect(slot);
}

void Stylist::on_enable() {
  apply_rules();
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

void Spire::Styles::set_style(QWidget& widget, const StyleSheet& style) {
  find_stylist(widget).set_style(style);
}

Block Spire::Styles::compute_style(QWidget& widget) {
  return find_stylist(widget).compute_style();
}

Block Spire::Styles::compute_style(
    QWidget& widget, const PseudoElement& pseudo_element) {
  if(auto stylist = find_stylist(widget, pseudo_element)) {
    return stylist->compute_style();
  }
  return {};
}

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
    return stylist->connect_style_signal(slot);
  }
  return {};
}
