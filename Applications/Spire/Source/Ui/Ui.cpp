#include "Spire/Ui/Ui.hpp"
#include <QPainter>
#include <QtSvg/QSvgRenderer>

using namespace Spire;

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  auto renderer = QSvgRenderer(path);
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  QPainter painter(&image);
  renderer.render(&painter, box);
  return image;
}
