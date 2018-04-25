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
      m_body(body) {
  this->::QWidget::window()->setWindowFlags(
    this->::QWidget::window()->windowFlags() | Qt::Window |
    Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  this->::QWidget::window()->setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<drop_shadow>(this);
  m_shadow->setMouseTracking(true);
  m_shadow->installEventFilter(this);
  resize(m_body->width(), m_body->height());
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
      if(m_resize_boxes.is_initialized()) {
        update_resize_boxes();
      }
    } else if(event->type() == QEvent::Move) {
      if(m_resize_boxes.is_initialized()) {
        update_resize_boxes();
      }
    }
  } else if(watched == m_shadow.get() || watched == m_border) {
    if(event->type() == QEvent::MouseMove) {
      update_resize_cursor();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::set_border_stylesheet(const QColor& color) {
  m_border->setStyleSheet(QString(R"(
    #window_border {
      border: %1px solid %3 %2px solid %3;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(color.name()));
}

void window::update_resize_boxes() {
  auto padding_size = scale(9, 9);
  auto top_left = mapToGlobal(geometry().topLeft());
  if(!m_resize_boxes.is_initialized()) {
    m_resize_boxes.emplace();
  }
  m_resize_boxes->m_top_left = QRect(top_left.x() - padding_size.width(),
    top_left.y() - padding_size.height(), padding_size.width(),
    padding_size.height());
  m_resize_boxes->m_top = QRect(top_left.x(),
    top_left.y() - padding_size.height(), width(), padding_size.height());
  m_resize_boxes->m_top_right = QRect(top_left.x() + width(),
    top_left.y() - padding_size.height(),
    padding_size.width(), padding_size.height());
  m_resize_boxes->m_right = QRect(top_left.x() + width(), top_left.y(),
    padding_size.width(), top_left.y() + height());
  m_resize_boxes->m_bottom_right = QRect(top_left.x() + width(),
    top_left.y() + height(), padding_size.width(), padding_size.height());
  m_resize_boxes->m_bottom = QRect(top_left.x(), top_left.y() + height(),
    width(), padding_size.height());
  m_resize_boxes->m_bottom_left = QRect(top_left.x() - padding_size.width(),
    top_left.y() + height(), padding_size.width(), padding_size.height());
  m_resize_boxes->m_left = QRect(top_left.x() - padding_size.width(),
    top_left.y(), padding_size.width(), top_left.y() + height());
}

void window::update_resize_cursor() {
  if(!m_resize_boxes.is_initialized()) {
    update_resize_boxes();
  }
  auto pos = QCursor::pos();
  auto cursor = qApp->overrideCursor()->shape();
  bool is_default_cursor = true;
  if(m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_right.contains(pos) ||
        m_resize_boxes->m_left.contains(pos)) {
      cursor = Qt::SizeHorCursor;
      is_default_cursor = false;
    }
  }
  if(m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_bottom.contains(pos) ||
        m_resize_boxes->m_top.contains(pos)) {
      cursor = Qt::SizeVerCursor;
      is_default_cursor = false;
      resize(size().width(), size().height() + 1);
      this->::QWidget::window()->resize(this->::QWidget::window()->size().width(), this->::QWidget::window()->size().height() + 1);
    }
  }
  if(m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed &&
      m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_bottom_right.contains(pos) ||
        m_resize_boxes->m_top_left.contains(pos)) {
      cursor = Qt::SizeFDiagCursor;
      is_default_cursor = false;
    } else if(m_resize_boxes->m_top_right.contains(pos) ||
        m_resize_boxes->m_bottom_left.contains(pos)) {
      cursor = Qt::SizeBDiagCursor;
      is_default_cursor = false;
    }
  }
  if(is_default_cursor) {
    cursor = Qt::ArrowCursor;
  }
  if(cursor != qApp->overrideCursor()->shape()) {
    qApp->restoreOverrideCursor();
    qApp->setOverrideCursor(QCursor(cursor));
  }
}
