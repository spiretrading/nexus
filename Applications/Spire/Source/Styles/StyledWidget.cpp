#include "Spire/Styles/StyledWidget.hpp"
#include <deque>
#include <set>
#include "Spire/Styles/VoidSelector.hpp"

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
        if(!test_selector(widget, element, selector.get_base())) {
          return false;
        }
        for(auto child : widget.children()) {
          if(auto c = qobject_cast<QWidget*>(child)) {
            if(test_selector(*c, element, selector.get_child())) {
              return true;
            }
          }
        }
        return false;
      },
      [&] (const IsASelector& selector) {
        return selector.is_instance(widget);
      },
      [&] {
        return selector.is_match(element);
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
  return compute_style(VoidSelector());
}

Block StyledWidget::compute_style(const Selector& element) const {
  auto block = Block();
  auto widget = static_cast<const QObject*>(this);
  auto excluded = std::set<std::type_index>();
  while(widget) {
    if(auto styled_widget = dynamic_cast<const StyledWidget*>(widget)) {
      for(auto& rule : styled_widget->m_style.get_rules()) {
        if(test_selector(element, rule.get_selector())) {
          if(excluded.empty()) {
            merge(block, rule.get_block());
          } else {
            for(auto& property : rule.get_block().get_properties()) {
              if(excluded.find(property.get_type()) == excluded.end()) {
                block.set(property);
              }
            }
          }
          if(rule.get_override() == Rule::Override::NONE) {
            return block;
          } else if(rule.get_override() == Rule::Override::EXCLUSIVE) {
            for(auto& property : rule.get_block().get_properties()) {
              excluded.insert(property.get_type());
            }
          }
        }
      }
    }
    widget = widget->parent();
  }
  return block;
}

bool StyledWidget::test_selector(const Selector& element,
    const Selector& selector) const {
  return base_test_selector(*this, element, selector);
}

void StyledWidget::style_updated() {
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
