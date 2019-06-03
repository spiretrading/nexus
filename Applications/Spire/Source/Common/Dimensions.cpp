#include "Spire/Spire/Dimensions.hpp"
#include <QApplication>
#include <QDesktopWidget>

using namespace Spire;

int Spire::scale_width(int width) noexcept {
  auto dpiX = QApplication::desktop()->logicalDpiX();
  return (width * dpiX) / DPI;
}

int Spire::scale_height(int height) noexcept {
  auto dpiY = QApplication::desktop()->logicalDpiY();
  return (height * dpiY) / DPI;
}

QSize Spire::scale(int width, int height) noexcept {
  return QSize(scale_width(width), scale_height(height));
}

QSize Spire::scale(const QSize& size) noexcept {
  return scale(size.width(), size.height());
}

QPoint Spire::translate(int x, int y) noexcept {
  return QPoint(scale_width(x), scale_height(y));
}

QPoint Spire::translate(const QPoint& p) noexcept {
  return translate(p.x(), p.y());
}
