#include "Spire/Ui/Box.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(scale_width(8)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

Box::Box(QWidget& body, QWidget* parent)
  : Box(&body, parent) {}

Box::Box(QWidget* parent)
  : Box(nullptr, parent) {}

void Box::enterEvent(QEvent* event) {
  update();
}

void Box::leaveEvent(QEvent* event) {
  update();
}

void Box::paintEvent(QPaintEvent* event) {
  auto computed_style = compute_style();
  auto style = QString("#Box {");
  style += "border-style: solid;";
  auto body_geometry = QRect(0, 0, width(), height());
  for(auto& property : computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        style += "background-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderTopSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-top-width: " + QString::number(computed_size) + "px;";
        body_geometry.setTop(body_geometry.top() + computed_size);
      },
      [&] (const BorderRightSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-right-width: " + QString::number(computed_size) +
          "px;";
        body_geometry.setRight(body_geometry.right() - computed_size);
      },
      [&] (const BorderBottomSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-bottom-width: " + QString::number(computed_size) +
          "px;";
        body_geometry.setBottom(body_geometry.bottom() - computed_size);
      },
      [&] (const BorderLeftSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-left-width: " + QString::number(computed_size) + "px;";
        body_geometry.setLeft(body_geometry.left() + computed_size);
      },
      [&] (const BorderTopColor& color) {
        style += "border-top-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderRightColor& color) {
        style += "border-right-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderBottomColor& color) {
        style += "border-bottom-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderLeftColor& color) {
        style += "border-left-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const PaddingTop& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-top: " + QString::number(computed_size) + "px;";
        body_geometry.setTop(body_geometry.top() + computed_size);
      },
      [&] (const PaddingRight& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-right: " + QString::number(computed_size) + "px;";
        body_geometry.setRight(body_geometry.right() - computed_size);
      },
      [&] (const PaddingBottom& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-bottom: " + QString::number(computed_size) + "px;";
        body_geometry.setBottom(body_geometry.bottom() - computed_size);
      },
      [&] (const PaddingLeft& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-left: " + QString::number(computed_size) + "px;";
        body_geometry.setLeft(body_geometry.left() + computed_size);
      },
      [] {});
  }
  style += "}";
  if(style != styleSheet()) {
    setStyleSheet(style);
    if(m_body) {
      m_body->setGeometry(body_geometry);
    }
  }
  StyledWidget::paintEvent(event);
}

Box::Box(QWidget* body, QWidget* parent)
  : StyledWidget(parent),
    m_body(body) {
  setObjectName("Box");
  set_style(DEFAULT_STYLE());
  if(m_body) {
    m_body->setParent(this);
    setFocusProxy(m_body);
  }
}
