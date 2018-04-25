#include "spire/ui/window.hpp"
#include <QApplication>
#include <QEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_first_show(true) {
  this->::QWidget::window()->setWindowFlags(
    this->::QWidget::window()->windowFlags() | Qt::Window |
    Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  this->::QWidget::window()->setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<drop_shadow>(this);
  m_shadow->setMouseTracking(true);
  m_shadow->installEventFilter(this);
  resize(m_body->width(), m_body->height());
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_border = new QWidget(this);
  m_border->setMouseTracking(true);
  m_border->installEventFilter(this);
  m_border->setObjectName("window_border");
  m_border->resize(m_body->size() + scale(1, 1));
  set_border_stylesheet("#A0A0A0");
  layout->addWidget(m_border);
  auto border_layout = new QVBoxLayout(m_border);
  border_layout->setMargin(scale_width(1));
  border_layout->setSpacing(0);
  m_title_bar = new title_bar(m_border);
  border_layout->addWidget(m_title_bar);
  border_layout->addWidget(m_body, 1);
  this->::QWidget::window()->installEventFilter(this);
  calculate_resize_rects();
  qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

bool window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == this->::QWidget::window()) {
    if(event->type() == QEvent::WindowActivate) {
      set_border_stylesheet("#A0A0A0");
      m_shadow->raise();
    } else if(event->type() == QEvent::WindowDeactivate) {
      set_border_stylesheet("#C8C8C8");
    } else if(event->type() == QEvent::Resize) {
      auto e = static_cast<QResizeEvent*>(event);
      if(e->size().height() > height()) {
        this->::QWidget::window()->resize(size());
      }
    }
  } else if(watched == m_shadow.get() || watched == m_border) {
    if(event->type() == QEvent::MouseMove) {
      auto pos = m_border->mapFromGlobal(QCursor::pos());
      auto cursor = qApp->overrideCursor()->shape();
      if(m_bottom_right_rect.contains(pos) || m_top_left_rect.contains(pos)) {
        cursor = Qt::SizeFDiagCursor;
      } else if(m_bottom_rect.contains(pos) || m_top_rect.contains(pos)) {
        cursor = Qt::SizeVerCursor;
      } else if(m_top_right_rect.contains(pos) ||
          m_bottom_left_rect.contains(pos)) {
        cursor = Qt::SizeBDiagCursor;
      } else if(m_right_rect.contains(pos) || m_left_rect.contains(pos)) {
        cursor = Qt::SizeHorCursor;
      } else {
        cursor = Qt::ArrowCursor;
      }
      if(cursor != qApp->overrideCursor()->shape()) {
        qApp->restoreOverrideCursor();
        qApp->setOverrideCursor(QCursor(cursor));
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::calculate_resize_rects() {
  auto padding_size = scale(9, 9);
  auto border = m_border->geometry();
  m_top_left_rect = QRect(border.topLeft().x() - padding_size.width(),
    border.topLeft().y() - padding_size.height(),
    padding_size.width(), padding_size.height());
  m_top_rect = QRect(border.x(), border.y() - padding_size.height(),
    border.width(), padding_size.height());
  m_top_right_rect = QRect(border.width(), border.y() - padding_size.height(),
    padding_size.width(), padding_size.height());
  m_right_rect = QRect(border.width(), border.y(),
    padding_size.width(), border.height() + m_title_bar->height());
  m_bottom_right_rect = QRect(border.width(),
    border.height() + m_title_bar->height(),
    padding_size.width(), padding_size.height());
  m_bottom_rect = QRect(border.x(), border.height() + m_title_bar->height(),
    border.width(), padding_size.height());
  m_bottom_left_rect = QRect(border.x() - padding_size.width(),
    border.height() + m_title_bar->height(),
    padding_size.width(), padding_size.height());
  m_left_rect = QRect(border.x() - padding_size.width(), border.y(),
    padding_size.width(), border.height() + m_title_bar->height());
}

void window::set_border_stylesheet(const QColor& color) {
  m_border->setStyleSheet(QString(R"(
    #window_border {
      border: %1px solid %3 %2px solid %3;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(color.name()));
}
