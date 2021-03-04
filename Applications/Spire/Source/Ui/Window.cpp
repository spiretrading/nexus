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
#include <Windows.h>
#include "Spire/Spire/Dimensions.hpp"
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

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26), QRect(translate(8, 8),
      scale(10, 10)));
  }
}

Window::Window(QWidget* parent)
    : QWidget(parent),
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

bool Window::nativeEvent(const QByteArray& eventType, void* message,
    long* result) {
  auto msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCACTIVATE) {
    RedrawWindow(msg->hwnd, NULL, NULL, RDW_UPDATENOW);
    *result = -1;
    return true;
  } else if(msg->message == WM_NCCALCSIZE) {
    if(static_cast<bool>(msg->wParam)) {
      if(!m_frame_size) {
        m_frame_size = size();
      }
      auto& rect = reinterpret_cast<NCCALCSIZE_PARAMS*>(
        msg->lParam)->rgrc[0];
      auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
      if (!IsZoomed(hwnd)) {
        rect.right -= GetSystemMetrics(SM_CXFRAME) +
          GetSystemMetrics(SM_CXPADDEDBORDER);
        rect.left += GetSystemMetrics(SM_CXFRAME) +
          GetSystemMetrics(SM_CXPADDEDBORDER);
        rect.bottom -= GetSystemMetrics(SM_CYFRAME) +
          GetSystemMetrics(SM_CXPADDEDBORDER);
        return true;
      }
      auto monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
      if (!monitor) {
        return false;
      }
      auto monitor_info = MONITORINFO{};
      monitor_info.cbSize = sizeof(monitor_info);
      if (::GetMonitorInfoW(monitor, &monitor_info)) {
        rect = monitor_info.rcWork;
      }
      *result = 0;
      return true;
    }
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT{};
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
    if(m_title_bar->get_title_label()->geometry().contains(pos)) {
      *result = HTCAPTION;
      return true;
    }
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam == SIZE_MAXIMIZED) {
      layout()->setContentsMargins({});
    } else if(msg->wParam == SIZE_RESTORED) {
      layout()->setContentsMargins(scale_width(1), scale_height(1), scale_width(1),
        scale_height(1));
    }
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    mmi->ptMaxTrackSize.x = maximumSize().width();
    mmi->ptMaxTrackSize.y = maximumSize().height();
    mmi->ptMinTrackSize.x = minimumSize().width();
    mmi->ptMinTrackSize.y = minimumSize().height();
    return true;
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void Window::resize_body(const QSize& size) {
  resize({size.width(), size.height() + m_title_bar->height()});
}

void Window::on_screen_changed(QScreen* screen) {
  // TODO: Workaround for this change:
  // https://github.com/qt/qtbase/commit/d2fd9b1b9818b3ec88487967e010f66e92952f55
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  auto rect = RECT{};
  GetWindowRect(hwnd, &rect);
  SendMessage(hwnd, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&rect));
}

void Window::set_fixed_body_size(const QSize& size) {
  set_window_attributes(false);
  setFixedSize({size.width(), size.height() + m_title_bar->height()});
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
  if(m_frame_size && size() != m_frame_size) {
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
