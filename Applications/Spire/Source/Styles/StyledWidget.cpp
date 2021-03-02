#include "Spire/Styles/StyledWidget.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
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
  for(auto& rule : m_style.get_rules()) {
    if(test_selector(rule.get_selector())) {
      merge(block, rule.get_block());
    }
  }
  return block;
}

bool StyledWidget::test_selector(const Selector& selector) const {
  try {
    return selector.visit(
      [&] (Any) {
        return true;
      },
      [&] (Active) {
        return isActiveWindow();
      },
      [&] (Disabled) {
        return !isEnabled();
      },
      [&] (Hovered) {
        return underMouse();
      },
      [&] (const NotSelector& selector) {
        return !test_selector(selector.get_selector());
      },
      [&] (const AndSelector& selector) {
        return test_selector(selector.get_left()) &&
          test_selector(selector.get_right());
      },
      [&] (const OrSelector& selector) {
        return test_selector(selector.get_left()) ||
          test_selector(selector.get_right());
      });
  } catch(const std::bad_any_cast&) {
    return false;
  }
}

void StyledWidget::style_updated() {
  update();
}
