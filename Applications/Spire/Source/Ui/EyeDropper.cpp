#include "Spire/Ui/EyeDropper.hpp"
#include <QApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QWindow>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto ZOOM_LEVEL = 11;
  const auto HALF_ZOOM_LEVEL = ZOOM_LEVEL / 2;

  auto grab_screens() {
    auto screens = QGuiApplication::screens();
    auto width = 0;
    auto height = 0;
    for(auto screen : screens) {
      auto geometry = screen->geometry();
      width += geometry.width();
      height = std::max(height, geometry.height());
    }
    auto screen_image = QPixmap(width, height);
    screen_image.fill(Qt::black);
    auto painter = QPainter(&screen_image);
    for(auto screen : screens) {
      painter.drawPixmap(screen->geometry().topLeft(), screen->grabWindow(0));
    }
    return screen_image.toImage();
  }
}

EyeDropper::EyeDropper(QWidget* parent)
    : QWidget(parent,
        Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
      m_current(std::make_shared<LocalColorModel>()),
      m_click_observer(*this),
      m_is_closed(false),
      m_screen_image(grab_screens()) {
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(scale(120, 120));
  setCursor(Qt::BlankCursor);
  qApp->installEventFilter(this);
  m_click_observer.connect_click_signal(
    std::bind_front(&EyeDropper::on_click, this));
  auto point = QCursor::pos();
  move(point.x() - width() / 2, point.y() - height() / 2);
  auto indicator_size = QSizeF(size()) / ZOOM_LEVEL;
  auto geometry = rect();
  auto center =
    QPointF(static_cast<double>(geometry.left() + geometry.right()) / 2,
      static_cast<double>(geometry.top() + geometry.bottom()) / 2);
  m_indicator_geometry = QRectF(QPointF(center.x() - indicator_size.width() / 2,
    center.y() - indicator_size.height() / 2), indicator_size);
  m_indicator_geometry.adjust(1, 1, 0, 0);
}

const std::shared_ptr<ValueModel<QColor>>& EyeDropper::get_current() const {
  return m_current;
}

connection EyeDropper::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection EyeDropper::connect_reject_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

bool EyeDropper::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::MouseMove) {
    if(m_is_closed) {
      return QWidget::eventFilter(watched, event);
    }
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    auto is_inside = false;
    auto windows = QApplication::topLevelWindows();
    for(auto window : windows) {
      if(window->isVisible() && window->isActive() &&
          window->winId() != winId()) {
        if(window->frameGeometry().contains(mouse_event.globalPos())) {
          is_inside = true;
        }
      }
    }
    if(is_inside) {
      show();
      move(mouse_event.globalX() - width() / 2,
        mouse_event.globalY() - height() / 2);
      m_current->set(m_screen_image.pixelColor(mouse_event.globalPos()));
      update();
    } else {
      hide();
    }
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

void EyeDropper::closeEvent(QCloseEvent* event) {
  m_is_closed = true;
  QWidget::closeEvent(event);
}

void EyeDropper::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    m_reject_signal(m_current->get());
    close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    m_submit_signal(m_current->get());
    close();
  }
  QWidget::keyPressEvent(event);
}

void EyeDropper::paintEvent(QPaintEvent* event) {
  auto geometry = rect();
  auto painter = QPainter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  auto path = QPainterPath();
  path.addEllipse(geometry);
  painter.setClipPath(path);
  auto position = QCursor::pos();
  painter.drawImage(geometry, m_screen_image,
    QRect(position.x() - HALF_ZOOM_LEVEL, position.y() - HALF_ZOOM_LEVEL,
      ZOOM_LEVEL, ZOOM_LEVEL));
  painter.setPen(QPen(QColor(0x808080), scale_width(1)));
  painter.drawEllipse(geometry.adjusted(1, 1, -1, -1));
  painter.drawRect(m_indicator_geometry);
}

void EyeDropper::on_click() {
  m_submit_signal(m_current->get());
  close();
}
