#include "spire/ui/window.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QVBoxLayout>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

namespace {
  const auto PADDING_SIZE = 10;
}

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_is_resizing(false),
      m_hovered(false) {
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
  m_title_bar = new title_bar(m_body, m_border);
  border_layout->addWidget(m_title_bar);
  border_layout->addWidget(m_body);
  this->::QWidget::window()->installEventFilter(this);
#ifdef Q_OS_WIN
  qApp->installNativeEventFilter(this);
#endif
}

void window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

#ifdef Q_OS_WIN
bool window::nativeEventFilter(const QByteArray& event_type, void* message,
    long* result) {
  auto msg = static_cast<MSG*>(message);
  if(msg->message == WM_GETMINMAXINFO) {
    auto maximize_dimensions = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    auto rect = QApplication::desktop()->availableGeometry(this);
    maximize_dimensions->ptMaxSize.x = rect.width();
    maximize_dimensions->ptMaxSize.y = rect.height() - 1;
    maximize_dimensions->ptMaxPosition.x = 0;
    maximize_dimensions->ptMaxPosition.y = 0;
    *result = 0;
    return true;
  }
  return false;
}
#endif

bool window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == this->::QWidget::window()) {
    if(event->type() == QEvent::WindowActivate) {
      set_border_stylesheet("#A0A0A0");
      m_shadow->raise();
      if(QWidget::window()->windowState().testFlag(Qt::WindowMinimized)) {
        QWidget::window()->setWindowState(Qt::WindowMaximized);
      }
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
  } else if(watched == m_shadow.get()) {
    if(event->type() == QEvent::MouseMove) {
      if(m_is_resizing && m_hovered) {
        handle_resize();
      } else {
        update_resize_cursor();
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      if(static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
        if(m_current_active_rect != active_resize_rect::NONE) {
          m_is_resizing = true;
        }
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      m_is_resizing = false;
      update_resize_cursor();
    } else if(event->type() == QEvent::Enter) {
      m_hovered = true;
    } else if(event->type() == QEvent::Leave) {
      m_hovered = false;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::handle_resize() {
  auto g = QWidget::window()->geometry();
  auto p = QCursor::pos();
  auto container_delta = g.size() - m_body->size();
  auto max_size = m_body->maximumSize() + container_delta;
  auto min_size = QSize(std::max(m_body->minimumWidth(), std::max(
    m_body->layout()->minimumSize().width(),
    std::max(m_title_bar->minimumWidth(),
    m_title_bar->layout()->minimumSize().width()))), m_body->minimumHeight()) +
    container_delta;
  if(m_current_active_rect == active_resize_rect::TOP ||
      m_current_active_rect == active_resize_rect::TOP_LEFT ||
      m_current_active_rect == active_resize_rect::TOP_RIGHT) {
    auto min_y = g.bottom() - max_size.height() + 1;
    auto max_y = g.bottom() - min_size.height() + 1;
    g.setY(std::max(min_y, std::min(max_y, p.y())));
  } else if(m_current_active_rect == active_resize_rect::BOTTOM ||
      m_current_active_rect == active_resize_rect::BOTTOM_LEFT ||
      m_current_active_rect == active_resize_rect::BOTTOM_RIGHT) {
    auto min_y = g.top() + min_size.height() - 1;
    auto max_y = g.top() + max_size.height() - 1;
    g.setBottom(std::max(min_y, std::min(max_y, p.y())));
  }
  if(m_current_active_rect == active_resize_rect::LEFT ||
      m_current_active_rect == active_resize_rect::TOP_LEFT ||
      m_current_active_rect == active_resize_rect::BOTTOM_LEFT) {
    auto min_x = g.right() - max_size.width() + 1;
    auto max_x = g.right() - min_size.width() + 1;
    g.setX(std::min(max_x, std::max(min_x, p.x())));
  } else if(m_current_active_rect == active_resize_rect::RIGHT ||
      m_current_active_rect == active_resize_rect::TOP_RIGHT ||
      m_current_active_rect == active_resize_rect::BOTTOM_RIGHT) {
    auto min_x = g.x() + min_size.width() - 1;
    auto max_x = g.x() + max_size.width() - 1;
    g.setRight(std::max(min_x, std::min(max_x, p.x())));
  }
  QWidget::window()->setGeometry(g);
}

void window::set_border_stylesheet(const QColor& color) {
  m_border->setStyleSheet(QString(R"(
    #window_border {
      border: %1px solid %3 %2px solid %3;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(color.name()));
}

void window::update_resize_boxes() {
  auto top_left = mapToGlobal(geometry().topLeft());
  if(!m_resize_boxes.is_initialized()) {
    m_resize_boxes.emplace();
  }
  m_resize_boxes->m_top_left = QRect(top_left.x() - PADDING_SIZE,
    top_left.y() - PADDING_SIZE, PADDING_SIZE, PADDING_SIZE);
  m_resize_boxes->m_top = QRect(top_left.x(), top_left.y() - PADDING_SIZE,
    width(), PADDING_SIZE);
  m_resize_boxes->m_top_right = QRect(top_left.x() + width(),
    top_left.y() - PADDING_SIZE, PADDING_SIZE, PADDING_SIZE);
  m_resize_boxes->m_right = QRect(top_left.x() + width(), top_left.y(),
    PADDING_SIZE, top_left.y() + height());
  m_resize_boxes->m_bottom_right = QRect(top_left.x() + width(),
    top_left.y() + height(), PADDING_SIZE, PADDING_SIZE);
  m_resize_boxes->m_bottom = QRect(top_left.x(), top_left.y() + height(),
    width(), PADDING_SIZE);
  m_resize_boxes->m_bottom_left = QRect(top_left.x() - PADDING_SIZE,
    top_left.y() + height(), PADDING_SIZE, PADDING_SIZE);
  m_resize_boxes->m_left = QRect(top_left.x() - PADDING_SIZE, top_left.y(),
    PADDING_SIZE, top_left.y() + height());
}

void window::update_resize_cursor() {
  if(QWidget::window()->windowState().testFlag(Qt::WindowMaximized)) {
    if(m_shadow->cursor().shape() != Qt::ArrowCursor) {
      m_shadow->setCursor(Qt::ArrowCursor);
    }
    m_current_active_rect = active_resize_rect::NONE;
    return;
  }
  if(!m_resize_boxes.is_initialized()) {
    update_resize_boxes();
  }
  auto pos = QCursor::pos();
  auto cursor = m_shadow->cursor().shape();
  m_current_active_rect = active_resize_rect::NONE;
  if(m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_right.contains(pos)) {
      cursor = Qt::SizeHorCursor;
      m_current_active_rect = active_resize_rect::RIGHT;
    } else if(m_resize_boxes->m_left.contains(pos)) {
      cursor = Qt::SizeHorCursor;
      m_current_active_rect = active_resize_rect::LEFT;
    }
  }
  if(m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_bottom.contains(pos)) {
      cursor = Qt::SizeVerCursor;
      m_current_active_rect = active_resize_rect::BOTTOM;
    } else if(m_resize_boxes->m_top.contains(pos)) {
      cursor = Qt::SizeVerCursor;
      m_current_active_rect = active_resize_rect::TOP;
    }
  }
  if(m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed &&
      m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
    if(m_resize_boxes->m_bottom_right.contains(pos)) {
      cursor = Qt::SizeFDiagCursor;
      m_current_active_rect = active_resize_rect::BOTTOM_RIGHT;
    } else if(m_resize_boxes->m_top_left.contains(pos)) {
      cursor = Qt::SizeFDiagCursor;
      m_current_active_rect = active_resize_rect::TOP_LEFT;
    } else if(m_resize_boxes->m_top_right.contains(pos)) {
      cursor = Qt::SizeBDiagCursor;
      m_current_active_rect = active_resize_rect::TOP_RIGHT;
    } else if(m_resize_boxes->m_bottom_left.contains(pos)) {
      cursor = Qt::SizeBDiagCursor;
      m_current_active_rect = active_resize_rect::BOTTOM_LEFT;
    }
  }
  if(m_current_active_rect == active_resize_rect::NONE) {
    cursor = Qt::ArrowCursor;
  }
  m_shadow->setCursor(cursor);
}
