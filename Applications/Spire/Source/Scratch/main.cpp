#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QBitmap>
#include <QLinearGradient>
#include <QPaintEngine>
#include <QWidget>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

namespace {
  QPointF get_curve_pos(QPointF p1, QPointF p2, QPointF p3, float p){    
    float x1 = (p2.x() - p1.x()) * p + p1.x();
    float y1 = (p2.y() - p1.y()) * p + p1.y();
    float x2 = (p3.x() - p2.x()) * p + p2.x();
    float y2 = (p3.y() - p2.y()) * p + p2.y();
    QPointF point = QPointF((x2 - x1) * p + x1, (y2 - y1) * p + y1);        
    return point;
  }

  // TODO: maybe update with and height to be the final size of the path,
  //        not the size of the widget.
  // assumes top side, which can be rotated into place
  // builds the path geometry without color
  QPainterPath create_border_side(int radius, int border_width, int width,
      int height, int previous_border_width, int next_border_width) {
    auto path = QPainterPath(QPointF(0, radius));
    //path.quadTo(0, 0, radius, 0);
    // TODO: fix potential off-by-ones with width and height
    //path.lineTo(width - radius, 0);
    //path.quadTo(width, 0, width, radius);
    //path.quadTo(width - border_width, border_width, width - radius, border_width);
    //path.lineTo(radius, border_width);
    //path.quadTo(border_width, border_width, 0, radius);


    //  if radius > 0
    //    
    //  else
    //    

    // if left border width is different
    //    if radius not 0
    //      create path as curved corner
    //    else
    //      create path as overlapped lines
    // else
    //    if radius is not 0
    //      create path as curved corner
    //    else
    //      create path as overlapped lines
    return path;
  }
}

class PaintTest : public QWidget {
  protected:
    void paintEvent(QPaintEvent* event) override {
      auto border_width = 10;
      auto radius = 20;
      auto painter = QPainter(this);
      painter.setPen(Qt::NoPen);
      // TODO: render onto QImage/QPixmap because otherwise the background of the parent
      //        is blended, too
      //painter.setCompositionMode(QPainter::CompositionMode_Plus);
      //painter.setRenderHint(QPainter::HighQualityAntialiasing);


      auto top = create_border_side(radius, border_width, width(), height(), 0, 0);
      painter.setBrush(QColor(Qt::red));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::green));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -height());
      painter.setBrush(QColor(Qt::blue));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::yellow));
      painter.drawPath(top);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->resize(250, 250);
  //w->setAttribute(Qt::WA_TranslucentBackground);
  auto l = new QHBoxLayout(w);
  auto pt = new PaintTest();
  pt->setFixedSize(100, 100);
  l->addWidget(pt);
  auto nw = new QWidget();
  nw->setStyleSheet(QString(R"(
    border: 20px solid red;
    border-radius: 25px;
    border-top-color: red;
    border-right-color: green;
    /*border-right: none;*/
    border-bottom-color: blue;
    border-left-color: yellow;
  )"));
  nw->setFixedSize(100, 100);
  l->addWidget(nw);
  w->show();
  application->exec();
}
