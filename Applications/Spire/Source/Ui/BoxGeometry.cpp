#include "Spire/Ui/BoxGeometry.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

BoxGeometry::BoxGeometry()
  : m_border_area(QPoint(0, 0), QSize(0, 0)),
    m_padding_area(QPoint(0, 0), QSize(0, 0)),
    m_content_area(QPoint(0, 0), QSize(0, 0)) {}

BoxGeometry::BoxGeometry(QSize size)
  : m_border_area(QPoint(0, 0), size),
    m_padding_area(QPoint(0, 0), size),
    m_content_area(QPoint(0, 0), size) {}

const QRect& BoxGeometry::get_geometry() const {
  return get_border_area();
}

const QRect& BoxGeometry::get_border_area() const {
  return m_border_area;
}

int BoxGeometry::get_border_top() const {
  return m_padding_area.top();
}

void BoxGeometry::set_border_top(int size) {
  auto padding = get_padding_top();
  m_padding_area.setTop(size);
  m_content_area.setTop(size + padding);
}

int BoxGeometry::get_border_right() const {
  return m_border_area.right() - m_padding_area.right();
}

void BoxGeometry::set_border_right(int size) {
  auto padding = get_padding_right();
  m_padding_area.setRight(m_border_area.right() - size);
  m_content_area.setRight(m_padding_area.right() - padding);
}

int BoxGeometry::get_border_bottom() const {
  return m_border_area.bottom() - m_padding_area.bottom();
}

void BoxGeometry::set_border_bottom(int size) {
  auto padding = get_padding_bottom();
  m_padding_area.setBottom(m_border_area.bottom() - size);
  m_content_area.setBottom(m_padding_area.bottom() - padding);
}

int BoxGeometry::get_border_left() const {
  return m_padding_area.left();
}

void BoxGeometry::set_border_left(int size) {
  auto padding = get_padding_left();
  m_padding_area.setLeft(size);
  m_content_area.setLeft(size + padding);
}

const QRect& BoxGeometry::get_padding_area() const {
  return m_padding_area;
}

int BoxGeometry::get_padding_top() const {
  return m_content_area.top() - m_padding_area.top();
}

void BoxGeometry::set_padding_top(int size) {
  m_content_area.setTop(m_padding_area.top() + size);
}

int BoxGeometry::get_padding_right() const {
  return m_padding_area.right() - m_content_area.right();
}

void BoxGeometry::set_padding_right(int size) {
  m_content_area.setRight(m_padding_area.right() - size);
}

int BoxGeometry::get_padding_bottom() const {
  return m_padding_area.bottom() - m_content_area.bottom();
}

void BoxGeometry::set_padding_bottom(int size) {
  m_content_area.setBottom(m_padding_area.bottom() - size);
}

int BoxGeometry::get_padding_left() const {
  return m_content_area.left() - m_padding_area.left();
}

void BoxGeometry::set_padding_left(int size) {
  m_content_area.setLeft(m_padding_area.left() + size);
}

const QRect& BoxGeometry::get_content_area() const {
  return m_content_area;
}

void BoxGeometry::set_size(QSize size) {
  auto right_border = get_border_right();
  auto bottom_border = get_border_bottom();
  auto right_padding = get_padding_right();
  auto bottom_padding = get_padding_bottom();
  m_border_area.setSize(size);
  m_padding_area.setRight(m_border_area.right() - right_border);
  m_padding_area.setBottom(m_border_area.bottom() - bottom_border);
  m_content_area.setRight(m_padding_area.right() - right_padding);
  m_content_area.setBottom(m_padding_area.bottom() - bottom_padding);
}

QSize Spire::get_styling_size(const BoxGeometry& geometry) {
  return geometry.get_geometry().size() - geometry.get_content_area().size();
}

void Spire::apply(
    const Property& property, BoxGeometry& geometry, Stylist& stylist) {
  property.visit(
    [&] (const BorderTopSize& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_border_top(size);
      });
    },
    [&] (const BorderRightSize& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_border_right(size);
      });
    },
    [&] (const BorderBottomSize& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_border_bottom(size);
      });
    },
    [&] (const BorderLeftSize& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_border_left(size);
      });
    },
    [&] (const PaddingTop& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_padding_top(size);
      });
    },
    [&] (const PaddingRight& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_padding_right(size);
      });
    },
    [&] (const PaddingBottom& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_padding_bottom(size);
      });
    },
    [&] (const PaddingLeft& size) {
      stylist.evaluate(size, [&geometry] (auto size) {
        geometry.set_padding_left(size);
      });
    });
}
