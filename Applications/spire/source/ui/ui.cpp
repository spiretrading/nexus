#include "spire/ui/ui.hpp"
#include <QLocale>
#include <QPainter>
#include <QtSvg/QSvgRenderer>

using namespace Spire;

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  QSvgRenderer renderer(path);
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  QPainter painter(&image);
  renderer.render(&painter, box);
  return image;
}

QString Spire::displayed_quantity(const Nexus::Quantity& quantity,
    int precision, const QLocale& locale) {
  auto str = locale.toString(static_cast<double>(quantity), 'f');
  auto index = str.indexOf(".");
  if(index == -1) {
    return str;
  }
  if(str.length() > index + precision) {
    str.chop(str.length() - (index + precision + 1));
  }
  for(auto i = str.length() - 1; i >= index; --i) {
    auto& ch = str.at(i);
    if(ch == "0" || ch == ".") {
      str.chop(1);
    } else {
      break;
    }
  }
  return str;
}
