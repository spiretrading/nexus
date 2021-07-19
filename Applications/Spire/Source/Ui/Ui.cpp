#include "Spire/Ui/Ui.hpp"
#include <QIcon>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

void Spire::draw_border(const QRect& region, const QColor& color,
    QPainter* painter) {
  painter->save();
  auto border_region = QRegion(region).subtracted(region.adjusted(
    scale_width(1), scale_height(1), -scale_width(1), -scale_height(1)));
  for(auto& rect : border_region) {
    painter->fillRect(rect, color);
  }
  painter->restore();
}

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

QSize Spire::get_border_size(const QWidget& widget) {
  auto border_size = QSize(0, 0);
  auto box = widget.findChild<Box*>("Box");
  if(!box) {
    return border_size;
  }
  for(auto& property : get_evaluated_block(*box)) {
    property.visit(
      [&] (std::in_place_type_t<BorderTopSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        border_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<BorderBottomSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        border_size.rwidth() += size;
      });
  }
  return border_size;
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

QHeaderView* Spire::make_fixed_header(QWidget* parent) {
  auto header = new QHeaderView(Qt::Horizontal, parent);
  header->setFixedHeight(scale_height(30));
  header->setStretchLastSection(true);
  header->setSectionsClickable(false);
  header->setSectionsMovable(false);
  header->setSectionResizeMode(QHeaderView::Fixed);
  header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  header->setStyleSheet(QString(R"(
    QHeaderView {
      background-color: #FFFFFF;
    }

    QHeaderView::section {
      background-color: #FFFFFF;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}

QHeaderView* Spire::make_header(QWidget* parent) {
  auto header = make_fixed_header(parent);
  header->setSectionsMovable(true);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setStyleSheet(QString(R"(
    QHeaderView {
      background-color: #FFFFFF;
      padding-left: %1px;
    }

    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}
