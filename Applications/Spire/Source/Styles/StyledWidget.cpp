#include "Spire/Styles/StyledWidget.hpp"
#include <deque>

using namespace Spire;
using namespace Spire::Styles;

namespace {
  bool base_test_selector(const QWidget& widget, const Selector& selector) {
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
        return widget.isEnabled() && widget.underMouse();
      },
      [&] (Focus) {
        return widget.hasFocus();
      },
      [&] (const NotSelector& selector) {
        return !test_selector(widget, selector.get_selector());
      },
      [&] (const AndSelector& selector) {
        return test_selector(widget, selector.get_left()) &&
          test_selector(widget, selector.get_right());
      },
      [&] (const OrSelector& selector) {
        return test_selector(widget, selector.get_left()) ||
          test_selector(widget, selector.get_right());
      },
      [&] (const AncestorSelector& selector) {
        if(!test_selector(widget, selector.get_base())) {
          return false;
        }
        auto p = widget.parentWidget();
        while(p != nullptr) {
          if(test_selector(*p, selector.get_ancestor())) {
            return true;
          }
          p = p->parentWidget();
        }
        return false;
      },
      [&] (const ParentSelector& selector) {
        if(!test_selector(widget, selector.get_base())) {
          return false;
        }
        if(auto p = widget.parentWidget()) {
          return test_selector(*p, selector.get_parent());
        }
        return false;
      },
      [&] (const DescendantSelector& selector) {
        if(!test_selector(widget, selector.get_base())) {
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
          if(test_selector(*descendant, selector.get_descendant())) {
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
        if(!test_selector(widget, selector.get_base())) {
          return false;
        }
        for(auto child : widget.children()) {
          if(auto c = qobject_cast<QWidget*>(child)) {
            if(test_selector(*c, selector.get_child())) {
              return true;
            }
          }
        }
        return false;
      },
      [&] (const IsASelector& selector) {
        return selector.is_instance(widget);
      },
      [] {
        return false;
      });
  }
}

StyledWidget::StyledWidget(QWidget* parent, Qt::WindowFlags flags)
  : StyledWidget({}, parent, flags) {}

StyledWidget::StyledWidget(StyleSheet style, QWidget* parent,
  Qt::WindowFlags flags)
  : m_style(std::move(style)),
    QWidget(parent, flags) {}

const StyleSheet& StyledWidget::get_style() const {
  return m_style;
}

void StyledWidget::set_style(const StyleSheet& style) {
  m_style = style;
  style_updated();
}

Block StyledWidget::compute_style() const {
  auto block = Block();
  auto widget = static_cast<const QObject*>(this);
  while(widget) {
    if(auto styled_widget = dynamic_cast<const StyledWidget*>(widget)) {
      for(auto& rule : styled_widget->m_style.get_rules()) {
        if(test_selector(rule.get_selector())) {
          merge(block, rule.get_block());
        }
      }
    }
    widget = widget->parent();
  }
  return block;
}

bool StyledWidget::test_selector(const Selector& selector) const {
  return base_test_selector(*this, selector);
}

void StyledWidget::style_updated() {
  update();
}

bool Spire::Styles::test_selector(const QWidget& widget,
    const Selector& selector) {
  if(auto styled_widget = dynamic_cast<const StyledWidget*>(&widget)) {
    return styled_widget->test_selector(selector);
  }
  return base_test_selector(widget, selector);
}
