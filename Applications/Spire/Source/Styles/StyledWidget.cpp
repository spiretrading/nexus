#include "Spire/Styles/StyledWidget.hpp"
#include <deque>
#include <QApplication>
#include <QEvent>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

struct StyledWidget::StyleEventFilter : QObject {
  StyledWidget* m_widget;

  StyleEventFilter(StyledWidget& widget)
      : QObject(&widget),
        m_widget(&widget) {
    connect(qApp, &QApplication::focusChanged, this,
      &StyleEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn) {
      m_widget->enable(Focus());
    } else if(event->type() == QEvent::FocusOut) {
      m_widget->disable(Focus());
    } else if(event->type() == QEvent::Enter) {
      m_widget->enable(Hover());
    } else if(event->type() == QEvent::Leave) {
      m_widget->disable(Hover());
    } else if(event->type() == QEvent::EnabledChange) {
      if(!m_widget->isEnabled()) {
        m_widget->enable(Disabled());
      } else {
        m_widget->disable(Disabled());
      }
    } else if(event->type() == QEvent::WindowActivate) {
      m_widget->enable(Active());
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_widget->disable(Active());
    }
    return QObject::eventFilter(watched, event);
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    auto proxy = m_widget->focusProxy();
    while(proxy) {
      if(proxy == now) {
        m_widget->enable(Focus());
        break;
      } else if(proxy == old) {
        m_widget->disable(Focus());
        break;
      }
      proxy = proxy->focusProxy();
    }
  }
};

std::size_t StyledWidget::SelectorHash::operator ()(
    const Selector& selector) const {
  return selector.get_type().hash_code();
}

bool StyledWidget::SelectorEquality::operator ()(
    const Selector& left, const Selector& right) const {
  return left.is_match(right);
}

StyledWidget::StyledWidget(QWidget* parent, Qt::WindowFlags flags)
  : StyledWidget({}, parent, flags) {}

StyledWidget::StyledWidget(StyleSheet style, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_style(std::move(style)),
      m_visibility(VisibilityOption::VISIBLE) {
  auto filter = new StyleEventFilter(*this);
  installEventFilter(filter);
}

StyledWidget::~StyledWidget() {
  for(auto dependent : m_dependents) {
    dependent->apply(*this, {});
  }
  while(!m_proxies.empty()) {
    remove_proxy(*m_proxies.front());
  }
  while(!m_principals.empty()) {
    m_principals.front()->remove_proxy(*this);
  }
}

const StyleSheet& StyledWidget::get_style() const {
  return m_style;
}

void StyledWidget::set_style(const StyleSheet& style) {
  m_style = style;
  m_enable_connections.clear();
  auto reach = build_reach(m_style, *this);
  for(auto widget : reach) {
    if(auto styled_widget = dynamic_cast<StyledWidget*>(widget)) {
      if(widget != this) {
        m_enable_connections.push_back(
          styled_widget->connect_enable_signal([=] { on_enable(); }));
      }
    }
  }
  apply_rules();
}

bool StyledWidget::is_enabled(const Selector& selector) const {
  if(m_enabled_selectors.find(selector) != m_enabled_selectors.end()) {
    return true;
  }
  for(auto proxy : m_proxies) {
    if(proxy->is_enabled(selector)) {
      return true;
    }
  }
  return false;
}

Block StyledWidget::compute_style() const {
  return compute_style(VoidSelector());
}

Block StyledWidget::compute_style(const Selector& element) const {
  auto block = Block();
  for(auto& entry : m_blocks) {
    merge(block, entry->m_block);
  }
  for(auto principal : m_principals) {
    merge(block, principal->compute_style(element));
  }
  return block;
}

void StyledWidget::add_proxy(QWidget& widget) {
  if(auto styled_widget = dynamic_cast<StyledWidget*>(&widget)) {
    auto i = std::find(m_proxies.begin(), m_proxies.end(), styled_widget);
    if(i == m_proxies.end()) {
      m_proxies.push_back(styled_widget);
      styled_widget->m_principals.push_back(this);
      styled_widget->apply_proxy_styles();
    }
  }
}

void StyledWidget::remove_proxy(QWidget& widget) {
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &widget);
  if(i == m_proxies.end()) {
    return;
  }
  auto styled_widget = *i;
  styled_widget->m_principals.erase(
    std::find(styled_widget->m_principals.begin(),
      styled_widget->m_principals.end(), this));
  m_proxies.erase(i);
  styled_widget->apply_proxy_styles();
}

void StyledWidget::enable(const Selector& selector) {
  if(m_enabled_selectors.insert(selector).second) {
    m_enable_signal();
    apply_rules();
    for(auto principal : m_principals) {
      principal->apply_rules();
    }
  }
}

void StyledWidget::disable(const Selector& selector) {
  if(m_enabled_selectors.erase(selector) != 0) {
    m_enable_signal();
    apply_rules();
    for(auto principal : m_principals) {
      principal->apply_rules();
    }
  }
}

void StyledWidget::apply_style() {
  auto style = compute_style();
  auto visibility_option = [&] {
    if(auto visibility = Spire::Styles::find<Visibility>(style)) {
      return visibility->get_expression().as<VisibilityOption>();
    }
    return VisibilityOption::VISIBLE;
  }();
  if(visibility_option != m_visibility) {
    if(visibility_option == VisibilityOption::VISIBLE) {
      show();
    } else if(visibility_option == VisibilityOption::NONE) {
      auto size = sizePolicy();
      size.setRetainSizeWhenHidden(false);
      setSizePolicy(size);
      hide();
    } else if(visibility_option == VisibilityOption::INVISIBLE) {
      auto size = sizePolicy();
      size.setRetainSizeWhenHidden(true);
      setSizePolicy(size);
      hide();
    }
    m_visibility = visibility_option;
  }
  update();
}

void StyledWidget::apply(const StyledWidget& source, Block block) {
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

void StyledWidget::apply_rules() {
  auto blocks = std::unordered_map<StyledWidget*, Block>();
  auto principals = std::deque<StyledWidget*>();
  principals.push_back(this);
  while(!principals.empty()) {
    auto principal = principals.front();
    principals.pop_front();
    principals.insert(principals.end(), principal->m_principals.begin(),
      principal->m_principals.end());
    for(auto& rule : principal->m_style.get_rules()) {
      auto selection = select(rule.get_selector(), *this);
      for(auto& selected : selection) {
        if(auto styled_widget = dynamic_cast<StyledWidget*>(selected)) {
          merge(blocks[styled_widget], rule.get_block());
        }
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
      previous_dependent->apply(*this, {});
    }
  }
  for(auto proxy : m_proxies) {
    proxy->apply_rules();
  }
}

void StyledWidget::apply_proxy_styles() {
  apply_style();
  for(auto proxy : m_proxies) {
    proxy->apply_proxy_styles();
  }
}

connection StyledWidget::connect_enable_signal(
    const EnableSignal::slot_type& slot) const {
  return m_enable_signal.connect(slot);
}

void StyledWidget::on_enable() {
  apply_rules();
}

std::vector<QWidget*> Spire::Styles::select(
    const Active& selector, QWidget& source) {
  if(source.isActiveWindow()) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Disabled& selector, QWidget& source) {
  if(!source.isEnabled()) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Hover& selector, QWidget& source) {
  if(source.isEnabled() &&
      source.rect().contains(source.mapFromGlobal(QCursor::pos()))) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Focus& selector, QWidget& source) {
  if(source.hasFocus()) {
    return {&source};
  }
  return {};
}
