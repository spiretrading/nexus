#include "Spire/Ui/EyeDropper.hpp"
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
      m_screen_image(grab_screens()),
      m_timer(this) {
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_TranslucentBackground);
  setCursor(Qt::BlankCursor);
  setMouseTracking(true);
  setFixedSize(scale(120, 120));
  m_click_observer.connect_click_signal(
    std::bind_front(&EyeDropper::on_click, this));
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

void EyeDropper::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    setMouseTracking(false);
    m_reject_signal(m_current->get());
    close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    setMouseTracking(false);
    m_submit_signal(m_current->get());
    close();
  }
  QWidget::keyPressEvent(event);
}

void EyeDropper::mouseMoveEvent(QMouseEvent* event) {
  move(event->globalPos());
  m_current->set(m_screen_image.pixelColor(event->globalPos()));
  update();
}

void EyeDropper::showEvent(QShowEvent* event) {
  move(QCursor::pos());
  auto indicator_size = QSizeF(size()) / ZOOM_LEVEL;
  auto geometry = rect();
  auto center =
    QPointF(static_cast<double>(geometry.left() + geometry.right()) / 2,
      static_cast<double>(geometry.top() + geometry.bottom()) / 2);
  m_indicator_geometry = QRectF(QPointF(center.x() - indicator_size.width() / 2,
    center.y() - indicator_size.height() / 2), indicator_size);
  m_indicator_geometry.adjust(1, 1, 0, 0);
  m_timer.setInterval(10);
  connect(&m_timer, &QTimer::timeout,
    std::bind_front(&EyeDropper::on_timeout, this));
  m_timer.start();
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

void EyeDropper::move(const QPoint& position) {
  QWidget::move(position.x() - width() / 2, position.y() - height() / 2);
}

void EyeDropper::on_click() {
  setMouseTracking(false);
  m_submit_signal(m_current->get());
  close();
}

void EyeDropper::on_timeout() {
  auto position = QCursor::pos();
  if(!rect().contains(mapFromGlobal(position))) {
    move(position);
  }
}
