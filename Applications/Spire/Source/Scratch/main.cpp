#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

void draw_border(QSize size, int left, int top, int radius, QWidget& widget) {
  auto painter = QPainter(&widget);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(QRect(QPoint(0, 0), size), Qt::white);
  auto top_left_bar = radius;
  painter.fillRect(
    QRect(QPoint(0, top_left_bar), QSize(left, size.height() - top_left_bar)),
    Qt::blue);
  auto corner_path = QPainterPath(QPoint(0, top_left_bar - 1));
  corner_path.arcTo(
    QRect(QPoint(0, 0), QSize(2 * radius - 1, 2 * radius - 1)), -180, -90);
  corner_path.lineTo(QPoint(radius, top));
  if(radius > top || radius > left) {
    corner_path.arcTo(QRect(QPoint(left, top),
      QSize(2 * (radius - left) - 1, 2 * (radius - top) - 1)), 90, 90);
  }
  corner_path.lineTo(QPoint(0, top_left_bar - 1));
  painter.setPen(Qt::blue);
  painter.fillPath(corner_path, Qt::blue);
//  painter.drawPath(corner_path);
  painter.fillRect(
    QRect(QPoint(radius, 0), QSize(size.width() - radius, top)),
    Qt::blue);
}

struct Canvas : QWidget {
  void paintEvent(QPaintEvent* event) override {
    draw_border(size(), scale_width(10), scale_height(20), scale_width(30),
      *this);
  };
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto canvas = new Canvas();
  canvas->resize(scale(1000, 1000));
  canvas->show();
  application->exec();
}
