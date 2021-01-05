#include "Spire/Ui/Window.hpp"
#include <QEvent>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TitleBar.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26), QRect(translate(8, 8),
      scale(10, 10)));
  }
}

Window::Window(QWidget* parent)
  : QWidget(parent),
  m_resize_area_width(scale_width(7)),
  m_shadow_margins(m_resize_area_width, m_resize_area_width,
    m_resize_area_width, m_resize_area_width),
  m_is_resizeable(true),
  m_title_bar(nullptr) {
  setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint |
    Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setObjectName("spire_window");
  m_title_bar = new TitleBar(make_svg_window_icon(":/Icons/spire.svg"), this);
  installEventFilter(m_title_bar);
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  m_central_widget = new QWidget(this);
  m_central_widget->setObjectName("central_widget");
  auto central_layout = new QVBoxLayout(this);
  central_layout->setSpacing(0);
  central_layout->setContentsMargins(0, 0, 0, 0);
  central_layout->addWidget(m_title_bar);
  m_central_widget->setLayout(central_layout);
  layout->addWidget(m_central_widget);
  auto shadow_effect = new QGraphicsDropShadowEffect(this);
  shadow_effect->setOffset(0, 0);
  m_central_widget->setGraphicsEffect(shadow_effect);
  setContentsMargins(m_shadow_margins);
  setLayout(layout);
}

void Window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void Window::set_svg_icon(const QString& icon_path) {
  set_icon(make_svg_window_icon(icon_path));
}

QLayout* Window::layout() const {
  return m_central_widget->layout();
}

void Window::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    auto shadow_effect = static_cast<QGraphicsDropShadowEffect*>(
      m_central_widget->graphicsEffect());
    if(isActiveWindow()) {
      shadow_effect->setColor(QColor(116, 120, 128));
      shadow_effect->setBlurRadius(22);
      m_central_widget->setStyleSheet(
        "#central_widget { background-color: #A0A0A0; }");
    } else {
      shadow_effect->setColor(Qt::lightGray);
      shadow_effect->setBlurRadius(18);
      m_central_widget->setStyleSheet(
        "#central_widget { background-color: #C8C8C8; }");
    }
  }
}

void Window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool Window::event(QEvent* event) {
  if(event->type() == QEvent::WinIdChange) {
    set_window_attributes(m_is_resizeable);
    connect(windowHandle(), &QWindow::screenChanged, this,
      &Window::on_screen_changed);
  }
  return QWidget::event(event);
}

bool Window::nativeEvent(const QByteArray& eventType, void* message,
  long* result) {
  auto msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCCALCSIZE) {
    result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT{};
    GetWindowRect(reinterpret_cast<HWND>(effectiveWinId()), &window_rect);
    auto x = GET_X_LPARAM(msg->lParam);
    auto y = GET_Y_LPARAM(msg->lParam);
    if(m_is_resizeable) {
      if(x >= window_rect.left &&
        x < window_rect.left + m_resize_area_width &&
        y < window_rect.bottom &&
        y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOMLEFT;
        return true;
      }
      if(x < window_rect.right &&
        x >= window_rect.right - m_resize_area_width &&
        y < window_rect.bottom &&
        y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOMRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
        x < window_rect.left + m_resize_area_width &&
        y >= window_rect.top &&
        y < window_rect.top + m_resize_area_width) {
        *result = HTTOPLEFT;
        return true;
      }
      if(x < window_rect.right &&
        x >= window_rect.right - m_resize_area_width &&
        y >= window_rect.top &&
        y < window_rect.top + m_resize_area_width) {
        *result = HTTOPRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
        x < window_rect.left + m_resize_area_width) {
        *result = HTLEFT;
        return true;
      }
      if(x < window_rect.right &&
        x >= window_rect.right - m_resize_area_width) {
        *result = HTRIGHT;
        return true;
      }
      if(y < window_rect.bottom &&
        y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOM;
        return true;
      }
      if(y >= window_rect.top &&
        y < window_rect.top + m_resize_area_width) {
        *result = HTTOP;
        return true;
      }
    }
    auto pos = m_title_bar->mapFromGlobal({x, y});
    if(m_title_bar->get_title_label()->geometry().contains(pos)) {
      *result = HTCAPTION;
      return true;
    }
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam == SIZE_MAXIMIZED) {
      auto abs_pos = mapToGlobal(m_title_bar->pos());
      auto pos = QGuiApplication::screenAt(mapToGlobal({window()->width() / 2,
        window()->height() / 2}))->geometry().topLeft();
      pos = QPoint(std::abs(abs_pos.x() - pos.x()),
        std::abs(abs_pos.y() - pos.y()));
      setContentsMargins(pos.x(), pos.y(), pos.x(), pos.y());
    } else if(msg->wParam == SIZE_RESTORED) {
      setContentsMargins(m_shadow_margins);
    }
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    mmi->ptMaxTrackSize.x = maximumSize().width();
    mmi->ptMaxTrackSize.y = maximumSize().height();
    mmi->ptMinTrackSize.x = minimumSize().width();
    mmi->ptMinTrackSize.y = minimumSize().height();
    return true;
  } else if(msg->message == WM_WINDOWPOSCHANGED) {
    auto screen_rect = screen()->availableGeometry();
    auto window_pos = reinterpret_cast<WINDOWPOS*>(msg->lParam);
    auto window_rect = QRect(window_pos->x, window_pos->y, window_pos->cx,
      window_pos->cy);
    if(window_rect.top() == screen_rect.top() &&
        window_rect.bottom() == screen_rect.bottom()) {
      if(window_rect.left() == screen_rect.left() ||
          window_rect.right() == screen_rect.right()) {
        m_shadow_margins = {0, 0, 0, 0};
      } else {
        m_shadow_margins = {m_resize_area_width, 0, m_resize_area_width, 0};
      }
    } else if(window_rect.height() == screen_rect.height() / 2 &&
        (window_rect.top() == screen_rect.top() ||
        window_rect.bottom() == screen_rect.bottom()) &&
        (window_rect.left() == screen_rect.left() ||
        window_rect.right() == screen_rect.right())) {
      m_shadow_margins = {0, 0, 0, 0};
    } else {
      m_shadow_margins = {m_resize_area_width, m_resize_area_width,
        m_resize_area_width, m_resize_area_width};
    }
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void Window::resize_body(const QSize& size) {
  resize({size.width() + m_shadow_margins.left() + m_shadow_margins.right(),
    size.height() + m_title_bar->height() + m_shadow_margins.top() +
    m_shadow_margins.bottom()});
}

void Window::on_screen_changed(QScreen* screen) {
  // TODO: Workaround for this change:
  // https://github.com/qt/qtbase/commit/d2fd9b1b9818b3ec88487967e010f66e92952f55
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  auto rect = RECT{0, 0, 1, 1};
  SendMessage(hwnd, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&rect));
}

void Window::set_fixed_body_size(const QSize& size) {
  set_window_attributes(false);
  setFixedSize({size.width() + m_shadow_margins.left() +
    m_shadow_margins.right(), size.height() + m_title_bar->height() +
    m_shadow_margins.top() + m_shadow_margins.bottom()});
}

void Window::set_window_attributes(bool is_resizeable) {
  m_is_resizeable = is_resizeable;
  if(m_is_resizeable &&
    maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)
      | WS_THICKFRAME | WS_CAPTION);
  } else if(m_is_resizeable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)
      | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX | WS_CAPTION);
  }
}
