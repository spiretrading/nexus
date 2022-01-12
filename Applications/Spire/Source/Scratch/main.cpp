#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

struct Border {
  int m_size;
  QColor m_color;
};

struct Borders {
  Border m_top;
  Border m_right;
  Border m_bottom;
  Border m_left;
  int m_top_right_radius;
  int m_bottom_right_radius;
  int m_bottom_left_radius;
  int m_top_left_radius;
};

void draw_border(QSize size, Borders borders, QWidget& widget) {
  auto painter = QPainter(&widget);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(QRect(QPoint(0, 0), size), Qt::white);
  painter.fillRect(QRect(QPoint(0, borders.m_top_left_radius - 1), QSize(
    borders.m_left.m_size, size.height() - borders.m_top_left_radius)),
    borders.m_left.m_color);
  auto top_left_corner_path =
    QPainterPath(QPoint(0, borders.m_top_left_radius - 1));
  top_left_corner_path.arcTo(QRect(QPoint(0, 0),
    QSize(2 * borders.m_top_left_radius, 2 * borders.m_top_left_radius)),
    -180, -90);
  top_left_corner_path.lineTo(
    QPoint(borders.m_top_left_radius, borders.m_top.m_size));
  if(borders.m_top_left_radius > borders.m_top.m_size ||
      borders.m_top_left_radius > borders.m_left.m_size) {
    top_left_corner_path.arcTo(
      QRect(QPoint(borders.m_left.m_size, borders.m_top.m_size),
      QSize(2 * (borders.m_top_left_radius - borders.m_left.m_size),
      2 * (borders.m_top_left_radius - borders.m_top.m_size))), 90, 90);
  }
  top_left_corner_path.lineTo(QPoint(0, borders.m_top_left_radius));
  painter.fillPath(top_left_corner_path, borders.m_top.m_color);
  painter.fillRect(
    QRect(QPoint(borders.m_top_left_radius, 0),
    QSize(size.width() - borders.m_top_left_radius, borders.m_top.m_size)),
    borders.m_top.m_color);
}

struct Canvas : QWidget {
  void paintEvent(QPaintEvent* event) override {
    auto borders = Borders();
    borders.m_left = {scale_width(10), Qt::blue};
    borders.m_top = {scale_height(20), Qt::blue};
    borders.m_top_left_radius = scale_width(30);
    draw_border(size(), borders, *this);
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
