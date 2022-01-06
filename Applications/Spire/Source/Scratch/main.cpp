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
#include "Spire/Ui/Box.hpp"
#include <QLabel>

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
  QPainterPath create_border_side(int left_radius, int right_radius,
      int border_width, int width,
      int height, int previous_border_width, int next_border_width) {
    // TODO: what if border_width is 0?

    // TODO: investigate potential off-by-ones with width and height

    // TODO: investigate if the precense of the left/right borders matter,
    //        it may be the case that 'sibling-less' borders will function
    //        properly with the same calculation that would be applied
    //        to a border that shares a corner.

    // **** LOC ****
    auto loc_start = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(0, left_radius);
      }
      // else, does share a left border
      auto point = QPointF();
      return point;
    }();
    auto loc_end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(left_radius, 0);
      }
      // else, does share a left border
      auto point = QPointF();
      return point;
    }();
    
    // **** ROC ****
    auto roc_start = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(width - right_radius, 0);
      }
      // shares border with right side
      return QPointF();
    }();
    auto roc_end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(width, right_radius);
      }
      // shares border with right side
      return QPointF();
    }();

    // **** RIC ****
    // Note: lines get reversed here, and the start is now on the right side of the end
    auto ric_start = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(width, right_radius);
      }
      // shares border with right side
      auto point = QPointF();
      return point;
    }();
    auto ric_ctrl = [&] () -> QPointF {
      return QPointF(width - next_border_width, border_width);
    }();
    auto ric_end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        // TODO: may not be right, may need to adjust (radius - width), see docs
        return QPointF(width - right_radius, border_width);
      }
      // shares border with right side
      auto point = QPointF();
      return point;
    }();

    // **** LIC ****
    auto lic_start = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width + left_radius, border_width);
      }
      // shares border with left side
      auto point = QPointF();
      return point;
    }();
    auto lic_ctrl = [&] () -> QPointF {
      return QPointF(previous_border_width, border_width);
    }();
    auto lic_end = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width, border_width + (left_radius - border_width));
      }
      // shares border with left side
      auto point = QPointF();
      return point;
    }();
    
    auto path = QPainterPath(loc_start);
    // fixed left outer control point
    path.quadTo(QPointF(0, 0), loc_end);
    path.lineTo(roc_start);
    // fixed right outer control point
    path.quadTo(QPointF(width, 0), roc_end);
    path.lineTo(ric_start);
    path.quadTo(ric_ctrl, ric_end);
    path.lineTo(lic_start);
    path.quadTo(lic_ctrl, lic_end);
    // close the path
    path.lineTo(loc_start);

    return path;
  }
}

class PaintTest : public QWidget {
  protected:
    void paintEvent(QPaintEvent* event) override {
      auto top_width = 40;
      auto right_width = 0;
      auto bottom_width = 40;
      auto left_width = 0;
      // TODO: these need to be manually reduced!
      auto tl_radius = 80;
      auto tr_radius = 120;
      auto br_radius = 80;
      auto bl_radius = 120;
      auto painter = QPainter(this);
      painter.fillRect(0, 0, width(), height(), QColor(0, 255, 255));
      painter.setPen(Qt::NoPen);
      // TODO: render onto QImage/QPixmap because otherwise the background of the parent
      //        is blended, too, or find another method
      //painter.setCompositionMode(QPainter::CompositionMode_Plus);
      //painter.setRenderHint(QPainter::HighQualityAntialiasing);

      painter.setBrush(QColor(Qt::red));
      painter.drawPath(create_border_side(tl_radius, tr_radius, top_width,
        width(), height(), left_width, right_width));

      painter.rotate(90);
      painter.translate(0, -width());
      //painter.setBrush(QColor(Qt::green));
      //painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -height());
      painter.setBrush(QColor(Qt::blue));
      painter.drawPath(create_border_side(br_radius, bl_radius, bottom_width,
        width(), height(), right_width, left_width));

      //painter.rotate(90);
      //painter.translate(0, -width());
      //painter.setBrush(QColor(Qt::yellow));
      //painter.drawPath(top);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->resize(350, 350);
  //w->setAttribute(Qt::WA_TranslucentBackground);
  auto l = new QHBoxLayout(w);
  auto pt = new PaintTest();
  pt->setFixedSize(200, 200);
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
  nw->setFixedSize(200, 200);
  l->addWidget(nw);
  //auto b = make_input_box(nullptr, w);
  //b->setFixedSize(100, 100);
  //l->addWidget(b);
  w->show();
  application->exec();
}
