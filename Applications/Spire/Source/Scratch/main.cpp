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
  double get_transition_point(int border_width1, int border_width2) {
    return static_cast<double>(border_width1) / (border_width1 + border_width2);
  }

  QPointF get_curve_pos(
      QPointF start, QPointF ctrl, QPointF end, double percent) {
    float x1 = (ctrl.x() - start.x()) * percent + start.x();
    float y1 = (ctrl.y() - start.y()) * percent + start.y();
    float x2 = (end.x() - ctrl.x()) * percent + ctrl.x();
    float y2 = (end.y() - ctrl.y()) * percent + ctrl.y();
    return {(x2 - x1) * percent + x1, (y2 - y1) * percent + y1};
  }

  struct Curve {
    QPointF m_start;
    QPointF m_ctrl;
    QPointF m_end;
  };

  Curve make_left_outer(int radius, int border, int previous_border) {
    auto start = [&] () -> QPointF {
      if(previous_border <= 0) {
        return QPointF(0, radius);
      }
      auto start = QPointF(0, radius);
      auto end = QPointF(radius, 0);
      auto tp = get_transition_point(border, previous_border);
      auto point = get_curve_pos(start, QPointF(0, 0), end, 1 - tp);
      return point;
    }();
    auto end = [&] () -> QPointF {
      return QPointF(radius, 0);
    }();
    return {start, QPointF(start.x(), end.y()), end};
  }

  Curve make_right_outer(int radius, int widget_width,
      int border_width, int next_border_width) {
    auto start = [&] () -> QPointF {
      return QPointF(widget_width - radius, 0);
    }();
    auto end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(widget_width, radius);
      }
      // shares border with right side
      auto start = QPointF(widget_width - radius, 0);
      auto end = QPointF(widget_width, radius);
      // TODO: transition points are shared between roc_end and ric_start
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_pos(start, QPointF(widget_width, 0), end, tp);
      return point;
    }();
    return {start, QPointF(end.x(), start.y()), end};
  }

  Curve make_right_inner(int radius, int widget_width,
      int border_width, int next_border_width) {
    // **** RIC ****
    // Note: lines get reversed here, and the start is now on the right side of the end
    auto start = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(widget_width, radius);
      }
      // shares border with right side
      auto start = QPointF(widget_width - next_border_width, radius);
      auto end = QPointF(widget_width - radius, border_width);
      auto tp = get_transition_point(border_width, next_border_width);
      // TODO: address this 1 - tp calculation by having a way from inferring
      //        direction in the get_transition_point function.
      auto point = get_curve_pos(start,
        QPointF(widget_width - next_border_width, border_width), end, 1 - tp);
      return point;
    }();
    auto ctrl = [&] () -> QPointF {
      return QPointF(widget_width - next_border_width, border_width);
    }();
    auto end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        // TODO: may not be right, may need to adjust (radius - width), see docs
        return QPointF(widget_width - radius, border_width);
      }
      return QPointF(
        widget_width - (next_border_width + radius - next_border_width),
        border_width);
    }();
    return {start, ctrl, end};
  }

  Curve make_left_inner(
      int radius, int border_width, int previous_border_width) {
    auto start = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width + radius, border_width);
      }
      // TODO: these curves are the same for each start/end of the inner corners,
      //        so don't duplicate
      // TODO: potentially constrain these radius - border calculations
      return QPointF(
        previous_border_width + radius - previous_border_width,
        border_width);
    }();
    auto ctrl = [&] () -> QPointF {
      return QPointF(previous_border_width, border_width);
    }();
    auto end = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(
          previous_border_width, border_width + (radius - border_width));
      }
      auto start = QPointF(
        previous_border_width + (radius - border_width),
        border_width);
      auto end = QPointF(previous_border_width,
        border_width + (radius - border_width));
      auto tp = get_transition_point(border_width, previous_border_width);
      auto point = get_curve_pos(
        start, QPointF(previous_border_width, border_width), end, tp);
      return point;
    }();
    return Curve{start, ctrl, end};
  }

  // assumes top side, which can be rotated into place
  // builds the path geometry without color
  QPainterPath create_border_side(int left_radius, int right_radius,
      int border_width, int width,
      int height, int previous_border_width, int next_border_width) {
    // TODO: if border_width is 0, still generate some border so the
    //        clip path works properly for the background.

    // TODO: investigate if the presence of the left/right borders matter,
    //        it may be the case that 'sibling-less' borders will function
    //        properly with the same calculation that would be applied
    //        to a border that shares a corner.

    // TODO: curves needs an additional ctrl point adjustment to match the
    //        connecting curve
    // TODO: ^^ this basically defines one bezier curve out of two,
    //        maybe write a function instead.

    auto loc = make_left_outer(
      left_radius, border_width, previous_border_width);
    auto roc = make_right_outer(right_radius, width,
      border_width, next_border_width);
    auto ric = make_right_inner(right_radius, width,
      border_width, next_border_width);
    auto lic = make_left_inner(left_radius,
      border_width, previous_border_width);
    auto path = QPainterPath(loc.m_start);
    path.quadTo(loc.m_ctrl, loc.m_end);
    path.lineTo(roc.m_start);
    path.quadTo(roc.m_ctrl, roc.m_end);
    path.lineTo(ric.m_start);
    path.quadTo(ric.m_ctrl, ric.m_end);
    path.lineTo(lic.m_start);
    path.quadTo(lic.m_ctrl, lic.m_end);
    path.lineTo(loc.m_start);
    return path;
  }
}

class PaintTest : public QWidget {
  protected:
    void paintEvent(QPaintEvent* event) override {
      auto top_width = 20;
      auto right_width = 20;
      auto bottom_width = 20;
      auto left_width = 20;
      // Note: these need to be manually reduced!
      auto tl_radius = 80;
      auto tr_radius = 40;
      auto br_radius = 80;
      auto bl_radius = 40;
      auto painter = QPainter(this);
      painter.fillRect(0, 0, width(), height(), QColor(0, 255, 255));
      painter.setPen(Qt::NoPen);
      // TODO: render onto QImage/QPixmap because otherwise the background of the parent
      //        is blended, too, or find another method
      //        Or, it appears to happen automatically when rendering paths.
      //painter.setCompositionMode(QPainter::CompositionMode_Plus);
      painter.setRenderHint(QPainter::HighQualityAntialiasing);

      painter.setBrush(QColor(Qt::red));
      painter.drawPath(create_border_side(tl_radius, tr_radius, top_width,
        width(), height(), left_width, right_width));

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::green));
      painter.drawPath(create_border_side(tr_radius, br_radius, right_width,
        height(), width(), top_width, bottom_width));

      painter.rotate(90);
      painter.translate(0, -height());
      painter.setBrush(QColor(Qt::blue));
      painter.drawPath(create_border_side(br_radius, bl_radius, bottom_width,
        width(), height(), right_width, left_width));

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::yellow));
      painter.drawPath(create_border_side(bl_radius, tl_radius, left_width,
        height(), width(), bottom_width, top_width));
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
