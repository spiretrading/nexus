#include "Spire/Styles/StyledWidget.hpp"
#include <deque>
#include <set>
#include <QLayout>
#include "Spire/Styles/SelectorRegistry.hpp"
#include "Spire/Styles/VoidSelector.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  bool base_test_selector(const QWidget& widget, const Selector& element,
      const Selector& selector) {
    return selector.visit(
      [&] (Any) {
        return true;
      },
      [&] (Active) {
        return widget.isActiveWindow();
      },
      [&] (Disabled) {
        return !widget.isEnabled();
      },
      [&] (Hover) {
        return widget.isEnabled() && widget.rect().contains(
          widget.mapFromGlobal(QCursor::pos()));
      },
      [&] (Focus) {
        return widget.hasFocus();
      },
      [&] (const NotSelector& selector) {
        return !test_selector(widget, element, selector.get_selector());
      },
      [&] (const AndSelector& selector) {
        return test_selector(widget, element, selector.get_left()) &&
          test_selector(widget, element, selector.get_right());
      },
      [&] (const OrSelector& selector) {
        return test_selector(widget, element, selector.get_left()) ||
          test_selector(widget, element, selector.get_right());
      },
      [&] (const AncestorSelector& selector) {
        if(!test_selector(widget, element, selector.get_base())) {
          return false;
        }
        auto p = widget.parentWidget();
        while(p != nullptr) {
          if(test_selector(*p, element, selector.get_ancestor())) {
            return true;
          }
          p = p->parentWidget();
        }
        return false;
      },
      [&] (const ParentSelector& selector) {
        if(!test_selector(widget, element, selector.get_base())) {
          return false;
        }
        if(auto p = widget.parentWidget()) {
          return test_selector(*p, element, selector.get_parent());
        }
        return false;
      },
      [&] (const DescendantSelector& selector) {
        if(!test_selector(widget, element, selector.get_base())) {
          return false;
        }
        auto descendants = std::deque<QWidget*>();
        for(auto child : widget.children()) {
          if(auto c = qobject_cast<QWidget*>(child)) {
            descendants.push_back(c);
          }
        }
        while(!descendants.empty()) {
          auto descendant = descendants.front();
          descendants.pop_front();
          if(test_selector(*descendant, element, selector.get_descendant())) {
            return true;
          }
          for(auto child : descendant->children()) {
            if(auto c = qobject_cast<QWidget*>(child)) {
              descendants.push_back(c);
            }
          }
        }
        return false;
      },
      [&] (const ChildSelector& selector) {
        auto parent = widget.parentWidget();
        if(!parent || !test_selector(widget, element, selector.get_child())) {
          return false;
        }
        return test_selector(*parent, element, selector.get_base());
      },
      [&] (const SiblingSelector& selector) {
        if(widget.parentWidget() == nullptr) {
          return false;
        }
        if(!test_selector(widget, element, selector.get_sibling())) {
          return false;
        }
        auto siblings = widget.parent()->children();
        auto i = 0;
        while(i != siblings.size()) {
          auto child = siblings[i];
          if(child != &widget) {
            if(auto c = qobject_cast<QWidget*>(child)) {
              if(test_selector(*c, element, selector.get_base())) {
                return true;
              }
            } else if(auto layout = qobject_cast<QLayout*>(child)) {
              siblings.append(layout->children());
            }
          }
          ++i;
        }
        return false;
      },
      [&] (const IsASelector& selector) {
        return selector.is_instance(widget);
      },
      [&] (const PropertyMatchSelector& selector) {
        if(auto styled_widget = dynamic_cast<const StyledWidget*>(&widget)) {
          auto block = styled_widget->compute_style();
          for(auto& property : block.get_properties()) {
            if(property == selector.get_property()) {
              return true;
            }
          }
        }
        return false;
      },
      [&] {
        return selector.is_match(element);
      });
  }
}

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
  SelectorRegistry::add(*this);
}

StyledWidget::~StyledWidget() {
  while(!m_destinations.empty()) {
    unpropagate_style(**m_destinations.begin());
  }
}

const StyleSheet& StyledWidget::get_style() const {
  return m_style;
}

void StyledWidget::set_style(const StyleSheet& style) {
  m_style = style;
  auto descendants = std::deque<QWidget*>();
  descendants.push_back(this);
  while(!descendants.empty()) {
    auto descendant = descendants.front();
    descendants.pop_front();
    if(auto styled_descendant = dynamic_cast<StyledWidget*>(descendant)) {
      styled_descendant->style_updated();
    }
    for(auto child : descendant->children()) {
      if(auto widget = dynamic_cast<QWidget*>(child)) {
        descendants.push_back(widget);
      }
    }
  }
}

Block StyledWidget::compute_style() const {
  return compute_style(VoidSelector());
}

Block StyledWidget::compute_style(const Selector& element) const {
  auto block = Block();
  for(auto& rule : m_style.get_rules()) {
    if(test_selector(element, rule.get_selector())) {
      for(auto& property : rule.get_block().get_properties()) {
        block.set(property);
      }
    }
  }
  for(auto& source : m_sources) {
    merge(block, source->compute_style(element));
  }
  return block;
}

void StyledWidget::propagate_style(StyledWidget& widget) {
  m_destinations.insert(&widget);
  widget.m_sources.insert(this);
}

void StyledWidget::unpropagate_style(StyledWidget& widget) {
  widget.m_sources.erase(this);
  m_destinations.erase(&widget);
}

bool StyledWidget::test_selector(const Selector& element,
    const Selector& selector) const {
  return base_test_selector(*this, element, selector);
}

void StyledWidget::enable(const Selector& selector) {
  if(m_enabled_selectors.insert(selector).second) {
    SelectorRegistry::find(*this).notify();
  }
}

void StyledWidget::disable(const Selector& selector) {
  auto i = m_enabled_selectors.find(selector);
  if(i != m_enabled_selectors.end()) {
    m_enabled_selectors.erase(i);
    SelectorRegistry::find(*this).notify();
  }
}

void StyledWidget::style_updated() {
  selector_updated();
}

void StyledWidget::selector_updated() {
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

bool Spire::Styles::test_selector(const QWidget& widget,
    const Selector& selector) {
  return test_selector(widget, VoidSelector(), selector);
}

bool Spire::Styles::test_selector(const QWidget& widget,
    const Selector& element, const Selector& selector) {
  if(auto styled_widget = dynamic_cast<const StyledWidget*>(&widget)) {
    return styled_widget->test_selector(element, selector);
  }
  return base_test_selector(widget, element, selector);
}
