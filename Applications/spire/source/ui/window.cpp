#include "spire/ui/window.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QVBoxLayout>
#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <qt_windows.h>
#endif
#include "spire/spire/dimensions.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

namespace {
  const auto PADDING_SIZE = 10;

  const auto TOP_STOPS = QGradientStops({
    QGradientStop(0.0, QColor(0, 0, 0, 43)),
    QGradientStop(0.05, QColor(0, 0, 0, 15)),
    QGradientStop(0.1, QColor(0, 0, 0, 8)),
    QGradientStop(0.20, QColor(0, 0, 0, 4)),
    QGradientStop(0.50, QColor(0, 0, 0, 1)),
    QGradientStop(1, Qt::transparent)});

  const auto BOTTOM_STOPS = QGradientStops({
    QGradientStop(0.0, QColor(0, 0, 0, 43)),
    QGradientStop(0.10, QColor(0, 0, 0, 15)),
    QGradientStop(0.16, QColor(0, 0, 0, 8)),
    QGradientStop(0.24, QColor(0, 0, 0, 4)),
    QGradientStop(0.50, QColor(0, 0, 0, 1)),
    QGradientStop(1, Qt::transparent)});

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26),
      QRect(translate(8, 8), scale(10, 10)));
  }
}

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_is_resizing(false) {
  QWidget::window()->setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_Hover);
  resize(m_body->width(), m_body->height());
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({scale_width(PADDING_SIZE),
    scale_height(PADDING_SIZE), scale_width(PADDING_SIZE),
    scale_height(PADDING_SIZE)});
  layout->setSpacing(0);
  m_border = new QWidget(this);
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
  QWidget::window()->installEventFilter(this);
  set_svg_icon(":icons/spire-icon-black.svg", ":icons/spire-icon-grey.svg");
  qApp->installNativeEventFilter(this);
}

void window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

void window::set_svg_icon(const QString& icon_path) {
  set_icon(make_svg_window_icon(icon_path));
  QWidget::window()->setWindowIcon(QIcon(icon_path));
}

void window::set_svg_icon(const QString& icon_path,
    const QString& unfocused_icon_path) {
  set_icon(make_svg_window_icon(icon_path),
    make_svg_window_icon(unfocused_icon_path));
  QWidget::window()->setWindowIcon(QIcon(icon_path));
}

#ifdef Q_OS_WIN
bool window::nativeEventFilter(const QByteArray& event_type, void* message,
    long* result) {
  auto msg = static_cast<::MSG*>(message);
  auto handle = reinterpret_cast<::HWND>(QWidget::window()->effectiveWinId());
  if(handle != msg->hwnd && !::IsChild(handle, msg->hwnd)) {
    return false;
  }
  if(msg->message == WM_ACTIVATE) {
    auto margins = ::MARGINS{-1, -1, -1, -1};
    ::DwmExtendFrameIntoClientArea(msg->hwnd, &margins);
    ::SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED |
      SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE |
      SWP_NOACTIVATE);
    return true;
  } else if((msg->message == WM_NCCALCSIZE) && msg->wParam == TRUE) {
    *result = 0;
    return true;
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto maximize_dimensions = reinterpret_cast<::MINMAXINFO*>(msg->lParam);
    auto rect = QApplication::desktop()->availableGeometry(this);
    maximize_dimensions->ptMaxSize.x = rect.width();
    maximize_dimensions->ptMaxSize.y = rect.height() - 1;
    maximize_dimensions->ptMaxPosition.x = 0;
    maximize_dimensions->ptMaxPosition.y = 0;
    *result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam != SIZE_MAXIMIZED) {
      auto g = QWidget::window()->frameGeometry();
      auto region = ::CreateRectRgn(0, 0, g.width(), g.height());
      ::SetWindowRgn(handle, region, FALSE);
    } else {
      ::SetWindowRgn(handle, NULL, FALSE);
    }
  } else if(msg->message == WM_NCPAINT) {
    *result = 0;
    return true;
  } else if(msg->message == WM_NCACTIVATE) {
    *result = TRUE;
    return true;
  } else if(msg->message == WM_SETTEXT) {
    *result = TRUE;
    return true;
  }
  return false;
}
#else
bool window::nativeEventFilter(const QByteArray& event_type, void* message,
    long* result) {
  return false;
}
#endif

bool window::event(QEvent* e) {
  if(e->type() == QEvent::HoverMove) {
    auto hover = static_cast<QHoverEvent*>(e);
    if(m_is_resizing) {
      handle_resize();
    } else {
      update_resize_cursor(hover->pos());
    }
    return true;
  } else if(e->type() == QEvent::HoverLeave) {
    m_current_active_rect = active_resize_rect::NONE;
    setCursor(Qt::ArrowCursor);
    return true;
  } else if(e->type() == QEvent::WindowActivate) {
    set_border_stylesheet("#A0A0A0");
  } else if(e->type() == QEvent::WindowDeactivate) {
    set_border_stylesheet("#C8C8C8");
  }
  return QWidget::event(e);
}

bool window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == QWidget::window()) {
    if(event->type() == QEvent::WindowStateChange) {
      if(QWidget::window()->isMaximized()) {
        m_normal_size = QWidget::window()->size();
        layout()->setContentsMargins({});
      } else {
        if(!QWidget::window()->isMinimized()) {
          QWidget::window()->resize(m_normal_size.width() + 1,
            m_normal_size.height());
          QWidget::window()->resize(m_normal_size);
        }
        layout()->setContentsMargins(
          {scale_width(PADDING_SIZE), scale_height(PADDING_SIZE),
          scale_width(PADDING_SIZE), scale_height(PADDING_SIZE)});
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    update_resize_cursor(event->pos());
    if(m_current_active_rect != active_resize_rect::NONE) {
      m_is_resizing = true;
    }
  }
}

void window::mouseReleaseEvent(QMouseEvent* event) {
  m_is_resizing = false;
  update_resize_cursor(event->pos());
}

void window::paintEvent(QPaintEvent* event) {
  if(QWidget::window()->isMaximized()) {
    return;
  }
  QPainter painter(this);
  auto parent_size = geometry().size() -
    QSize(2 * scale_width(PADDING_SIZE), 2 * scale_height(PADDING_SIZE));
  auto shadow_size = scale(PADDING_SIZE, PADDING_SIZE);
  auto right_start = shadow_size.width() + parent_size.width();
  auto bottom_start = shadow_size.height() + parent_size.height();
  QRect top_left_rect(QPoint(0, 0), shadow_size);
  QRect top_rect(QPoint(shadow_size.width(), 0),
    QSize(parent_size.width(), shadow_size.height()));
  QRadialGradient top_left_gradient(top_left_rect.bottomRight(),
    top_left_rect.width());
  top_left_gradient.setStops(TOP_STOPS);
  painter.fillRect(top_left_rect, top_left_gradient);
  QLinearGradient top_gradient(top_rect.bottomLeft(), top_rect.topLeft());
  top_gradient.setStops(TOP_STOPS);
  painter.fillRect(top_rect, top_gradient);
  auto top_right_rect = top_left_rect;
  top_right_rect.translate(right_start, 0);
  QRadialGradient top_right_gradient(top_right_rect.bottomLeft(),
    top_right_rect.width());
  top_right_gradient.setStops(TOP_STOPS);
  painter.fillRect(top_right_rect, top_right_gradient);
  auto bottom_left_rect = top_left_rect;
  bottom_left_rect.translate(0, bottom_start);
  QRadialGradient bottom_left_gradient(bottom_left_rect.topRight(),
    bottom_left_rect.width());
  bottom_left_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_left_rect, bottom_left_gradient);
  auto bottom_rect = top_rect;
  bottom_rect.translate(0, bottom_start);
  QLinearGradient bottom_gradient(bottom_rect.topLeft(),
    bottom_rect.bottomLeft());
  bottom_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_rect, bottom_gradient);
  auto bottom_right_rect = top_left_rect;
  bottom_right_rect.translate(right_start, bottom_start);
  QRadialGradient bottom_right_gradient(bottom_right_rect.topLeft(),
    bottom_right_rect.width());
  bottom_right_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(bottom_right_rect, bottom_right_gradient);
  QRect left_rect(QPoint(0, shadow_size.height()),
    QSize(shadow_size.width(), parent_size.height()));
  QLinearGradient left_gradient(left_rect.topRight(), left_rect.topLeft());
  left_gradient.setStops(TOP_STOPS);
  painter.fillRect(left_rect, left_gradient);
  auto right_rect = left_rect;
  right_rect.translate(right_start, 0);
  QLinearGradient right_gradient(right_rect.topLeft(), right_rect.topRight());
  right_gradient.setStops(BOTTOM_STOPS);
  painter.fillRect(right_rect, right_gradient);
  QWidget::paintEvent(event);
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
    container_delta + QSize(4, 0);
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

void window::update_resize_cursor(const QPoint& pos) {
  m_current_active_rect = active_resize_rect::NONE;
  if(!QWidget::window()->isMaximized() &&
      (m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed ||
      m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed)) {
    auto resize_width = scale_width(PADDING_SIZE);
    if(pos.x() <= resize_width &&
        m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
      m_current_active_rect |= active_resize_rect::LEFT;
    } else if(pos.x() >= width() - resize_width &&
        m_body->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
      m_current_active_rect |= active_resize_rect::RIGHT;
    }
    auto resize_height = scale_height(PADDING_SIZE);
    if(pos.y() <= resize_height &&
        m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed) {
      m_current_active_rect |= active_resize_rect::TOP;
    } else if(pos.y() >= height() - resize_height &&
        m_body->sizePolicy().verticalPolicy() != QSizePolicy::Fixed) {
      m_current_active_rect |= active_resize_rect::BOTTOM;
    }
  }
  auto cursor_shape = [&] {
    if(m_current_active_rect == active_resize_rect::TOP) {
      return Qt::SizeVerCursor;
    } else if(m_current_active_rect == active_resize_rect::TOP_RIGHT) {
      return Qt::SizeBDiagCursor;
    } else if(m_current_active_rect == active_resize_rect::RIGHT) {
      return Qt::SizeHorCursor;
    } else if(m_current_active_rect == active_resize_rect::BOTTOM_RIGHT) {
      return Qt::SizeFDiagCursor;
    } else if(m_current_active_rect == active_resize_rect::BOTTOM) {
      return Qt::SizeVerCursor;
    } else if(m_current_active_rect == active_resize_rect::BOTTOM_LEFT) {
      return Qt::SizeBDiagCursor;
    } else if(m_current_active_rect == active_resize_rect::LEFT) {
      return Qt::SizeHorCursor;
    } else if(m_current_active_rect == active_resize_rect::TOP_LEFT) {
      return Qt::SizeFDiagCursor;
    }
    return Qt::ArrowCursor;
  }();
  setCursor(cursor_shape);
}
