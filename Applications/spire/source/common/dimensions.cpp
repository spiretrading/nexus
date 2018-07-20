#include "spire/spire/dimensions.hpp"
#include <QApplication>
#include <QDesktopWidget>

using namespace spire;

int spire::scale_width(int width) noexcept {
  auto dpiX = QApplication::desktop()->logicalDpiX();
  return (width * dpiX) / DPI;
}

int spire::scale_height(int height) noexcept {
  auto dpiY = QApplication::desktop()->logicalDpiY();
  return (height * dpiY) / DPI;
}

QSize spire::scale(int width, int height) noexcept {
  return QSize(scale_width(width), scale_height(height));
}

QSize spire::scale(const QSize& size) noexcept {
  return scale(size.width(), size.height());
}

QPoint spire::translate(int x, int y) noexcept {
  return QPoint(scale_width(x), scale_height(y));
}

QPoint spire::translate(const QPoint& p) noexcept {
  return translate(p.x(), p.y());
}
