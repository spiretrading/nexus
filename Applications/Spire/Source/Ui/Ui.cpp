#include "Spire/Ui/Ui.hpp"
#include <QIcon>
#include <QLayout>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::posix_time;
using namespace Spire;

QPropertyAnimation* Spire::fade_window(QObject* target, bool reverse,
    time_duration fade_speed) {
  auto animation = new QPropertyAnimation(target, "windowOpacity", target);
  animation->setDuration(fade_speed.total_milliseconds());
  animation->setEasingCurve(QEasingCurve::Linear);
  if(!reverse) {
    animation->setStartValue(0);
    animation->setEndValue(1);
  } else {
    animation->setStartValue(1);
    animation->setEndValue(0);
  }
  animation->start(QAbstractAnimation::DeleteWhenStopped);
  return animation;
}

QWidget* Spire::find_focus_proxy(QWidget& widget) {
  auto proxy = &widget;
  while(proxy->focusProxy()) {
    proxy = proxy->focusProxy();
  }
  return proxy;
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  // TODO: Revert this when Qt fixes the regression.
  // https://bugreports.qt.io/browse/QTBUG-81259
  auto svg_pixmap = QIcon(path).pixmap(box.width(), box.height());
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  auto painter = QPainter(&image);
  painter.drawPixmap(box.topLeft(), svg_pixmap);
  return image;
}

void Spire::invalidate_descendant_layouts(QWidget& widget) {
  for(auto child : widget.children()) {
    if(!child->isWidgetType()) {
      continue;
    }
    auto& widget = *static_cast<QWidget*>(child);
    invalidate_descendant_layouts(widget);
    widget.updateGeometry();
    if(widget.layout()) {
      widget.layout()->invalidate();
    }
  }
}

int Spire::get_character_width(const QFont& font) {
  return QFontMetrics(font).horizontalAdvance('0');
}
