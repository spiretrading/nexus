#include "Spire/Ui/Window.hpp"
#include <QEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TitleBar.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto RESIZE_AREA() {
    return scale(8, 8);
  }

  auto ICON_SIZE() {
    return scale(26, 26);
  }

  auto SYSTEM_BORDER_SIZE() {
    static auto size = QSize(
      GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER),
      GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER));
    return size;
  }

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26), QRect(translate(8, 8),
      scale(10, 10)));
  }
}

Window::Window(QWidget* parent)
    : QWidget(parent),
      m_body(nullptr),
      m_is_resizable(true) {
  setWindowFlags(windowFlags() | Qt::Window | Qt::WindowSystemMenuHint);
  setObjectName("spire_window");
  m_title_bar = new TitleBar(make_svg_window_icon(":/Icons/spire.svg"), this);
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(1), scale_height(1), scale_width(1),
    scale_height(1));
  layout->addWidget(m_title_bar);
}

void Window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void Window::set_svg_icon(const QString& icon_path) {
  set_icon(make_svg_window_icon(icon_path));
}

void Window::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    if(isActiveWindow()) {
      setStyleSheet("#spire_window { background-color: #A0A0A0; }");
    } else {
      setStyleSheet("#spire_window { background-color: #C8C8C8; }");
    }
  }
}

void Window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool Window::event(QEvent* event) {
  if(event->type() == QEvent::WinIdChange) {
    set_window_attributes(m_is_resizable);
    connect(windowHandle(), &QWindow::screenChanged, this,
      &Window::on_screen_changed);
  }
  return QWidget::event(event);
}

bool Window::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    if(watched == m_body && !isMaximized()) {
      resize(adjusted_window_size(m_body->size()));
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool Window::nativeEvent(const QByteArray& eventType, void* message,
    long* result) {
  auto msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCACTIVATE) {
    RedrawWindow(msg->hwnd, NULL, NULL, RDW_UPDATENOW);
    *result = -1;
    return true;
  } else if(msg->message == WM_NCCALCSIZE) {
    if(msg->wParam != 0) {
      if(!m_frame_size) {
        m_frame_size = size();
      }
      auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
      auto placement = WINDOWPLACEMENT();
      if(GetWindowPlacement(hwnd, &placement)) {
        auto& rect = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam)->rgrc[0];
        if(IsZoomed(hwnd) ||
            (IsIconic(hwnd) && placement.flags & WPF_RESTORETOMAXIMIZED)) {
          if(auto monitor = MonitorFromRect(
              &placement.rcNormalPosition, MONITOR_DEFAULTTONEAREST)) {
            auto monitor_info = MONITORINFO();
            monitor_info.cbSize = sizeof(monitor_info);
            if(GetMonitorInfoW(monitor, &monitor_info)) {
              rect = monitor_info.rcWork;
            }
          }
        } else {
          rect.right -= SYSTEM_BORDER_SIZE().width();
          rect.left += SYSTEM_BORDER_SIZE().width();
          rect.bottom -= SYSTEM_BORDER_SIZE().height();
        }
      }
      *result = 0;
      return true;
    }
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT();
    GetWindowRect(reinterpret_cast<HWND>(effectiveWinId()), &window_rect);
    auto x = GET_X_LPARAM(msg->lParam);
    auto y = GET_Y_LPARAM(msg->lParam);
    auto resize_area = RESIZE_AREA();
    if(m_is_resizable) {
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width() &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOMLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width() &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOMRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width() &&
          y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOPLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width() &&
          y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOPRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width()) {
        *result = HTLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width()) {
        *result = HTRIGHT;
        return true;
      }
      if(y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOM;
        return true;
      }
      if(y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOP;
        return true;
      }
    }
    auto pos = m_title_bar->mapFromGlobal({x, y});
    if(m_title_bar->get_title_label().geometry().contains(pos)) {
      *result = HTCAPTION;
      return true;
    }
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam == SIZE_MAXIMIZED) {
      layout()->setContentsMargins({});
    } else if(msg->wParam == SIZE_RESTORED) {
      layout()->setContentsMargins(scale_width(1), scale_height(1),
        scale_width(1), scale_height(1));
    }
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    if(maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
      mmi->ptMaxTrackSize.x = maximumSize().width() +
        2 * SYSTEM_BORDER_SIZE().width();
      mmi->ptMaxTrackSize.y = maximumSize().height() +
        SYSTEM_BORDER_SIZE().height();
    } else {
      mmi->ptMaxTrackSize.x = maximumSize().width();
      mmi->ptMaxTrackSize.y = maximumSize().height();
    }
    mmi->ptMinTrackSize.x = minimumSize().width() +
      2 * SYSTEM_BORDER_SIZE().width();
    mmi->ptMinTrackSize.y = minimumSize().height() +
      SYSTEM_BORDER_SIZE().height();
    return true;
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void Window::set_body(QWidget* body) {
  if(m_body) {
    return;
  }
  m_body = body;
  if(body->maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
    set_window_attributes(false);
  } else {
    resize(adjusted_window_size(body->size()));
  }
  layout()->addWidget(m_body);
  m_body->installEventFilter(this);
}

QSize Window::adjusted_window_size(const QSize& body_size) {
  return {body_size.width() + 2 * scale_width(1),
    body_size.height() + m_title_bar->height() + 2 * scale_height(1)};
}

void Window::on_screen_changed(QScreen* screen) {
  // TODO: Workaround for this change:
  // https://github.com/qt/qtbase/commit/d2fd9b1b9818b3ec88487967e010f66e92952f55
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  auto rect = RECT();
  GetWindowRect(hwnd, &rect);
  SendMessage(hwnd, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&rect));
}

void Window::set_window_attributes(bool is_resizeable) {
  m_is_resizable = is_resizeable;
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  if(m_is_resizable &&
      maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
      WS_THICKFRAME | WS_CAPTION);
  } else if(m_is_resizable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
      WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX | WS_CAPTION);
  }
  if(m_is_resizable && m_frame_size && size() != m_frame_size) {
    resize(*m_frame_size);
    m_frame_size = none;
  }
  auto window_geometry = frameGeometry();
  auto borderWidth = GetSystemMetrics(SM_CXFRAME) +
    GetSystemMetrics(SM_CXPADDEDBORDER);
  auto borderHeight = GetSystemMetrics(SM_CYFRAME) +
    GetSystemMetrics(SM_CXPADDEDBORDER);
  auto internal_height = layout()->contentsRect().height();
  MoveWindow(hwnd, window_geometry.x() - borderWidth + 1,
    window_geometry.y() + borderHeight + m_title_bar->height() -
      scale_height(3),
    width() + 2 * borderWidth, internal_height + borderHeight, true);
}