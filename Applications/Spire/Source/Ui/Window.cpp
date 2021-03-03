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

  auto maximized(HWND hwnd) -> bool {
    auto placement = WINDOWPLACEMENT{};
    if (!::GetWindowPlacement(hwnd, &placement)) {
      return false;
    }
    return placement.showCmd == SW_MAXIMIZE;
  }

  auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void {
    if (!maximized(window)) {
      rect.right -= GetSystemMetrics(SM_CXFRAME) +
        GetSystemMetrics(SM_CXPADDEDBORDER);
      rect.left += GetSystemMetrics(SM_CXFRAME) +
        GetSystemMetrics(SM_CXPADDEDBORDER);
      rect.bottom -= GetSystemMetrics(SM_CYFRAME) +
        GetSystemMetrics(SM_CXPADDEDBORDER);
      return;
    }
    auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
    if (!monitor) {
      return;
    }
    auto monitor_info = MONITORINFO{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (::GetMonitorInfoW(monitor, &monitor_info)) {
      rect = monitor_info.rcWork;
    }
  }

  auto hit_test(HWND hwnd, POINT cursor) -> LRESULT {
    // identify borders and corners to allow resizing the window.
    // Note: On Windows 10, windows behave differently and
    // allow resizing outside the visible window frame.
    // This implementation does not replicate that behavior.
    const POINT border{
        ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
        ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
    };
    auto window_rect = RECT{};
    if (!::GetWindowRect(hwnd, &window_rect)) {
        return HTNOWHERE;
    }

    const auto drag = true ? HTCAPTION : HTCLIENT;

    enum region_mask {
        client = 0b0000,
        left   = 0b0001,
        right  = 0b0010,
        top    = 0b0100,
        bottom = 0b1000,
    };

    const auto result =
        left    * (cursor.x <  (window_rect.left   + border.x)) |
        right   * (cursor.x >= (window_rect.right  - border.x)) |
        top     * (cursor.y <  (window_rect.top    + border.y)) |
        bottom  * (cursor.y >= (window_rect.bottom - border.y));

    switch (result) {
        case left          : return true ? HTLEFT        : drag;
        case right         : return true ? HTRIGHT       : drag;
        case top           : return true ? HTTOP         : drag;
        case bottom        : return true ? HTBOTTOM      : drag;
        case top | left    : return true ? HTTOPLEFT     : drag;
        case top | right   : return true ? HTTOPRIGHT    : drag;
        case bottom | left : return true ? HTBOTTOMLEFT  : drag;
        case bottom | right: return true ? HTBOTTOMRIGHT : drag;
        case client        : return drag;
        default            : return HTNOWHERE;
    }
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
      setStyleSheet("#spire_window { background-color: red; }");
    } else {
      setStyleSheet("#spire_window { background-color: aqua; }");
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
      auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
      adjust_maximized_client_rect(msg->hwnd, params.rgrc[0]);
      *result = 0;
      return true;
    }
  } else if(msg->message == WM_NCHITTEST) {
    *result = hit_test(reinterpret_cast<HWND>(effectiveWinId()),
      POINT{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)});
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam == SIZE_MAXIMIZED) {
      auto abs_pos = mapToGlobal(m_title_bar->pos());
      auto pos = QGuiApplication::screenAt(mapToGlobal({window()->width() / 2,
        window()->height() / 2}))->geometry().topLeft();
      pos = QPoint(std::abs(abs_pos.x() - pos.x()),
        std::abs(abs_pos.y() - pos.y()));
      layout()->setContentsMargins({});
    } else if(msg->wParam == SIZE_RESTORED) {
      layout()->setContentsMargins(scale_width(1), scale_height(1), scale_width(1),
        scale_height(1));
      setContentsMargins({});
    }
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    mmi->ptMaxTrackSize.x = maximumSize().width();
    mmi->ptMaxTrackSize.y = maximumSize().height();
    mmi->ptMinTrackSize.x = minimumSize().width();
    mmi->ptMinTrackSize.y = minimumSize().height();
    return true;
  } else if(msg->message == WM_NCPAINT) {
    //auto hdc = HDC{};
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    //hdc = GetDCEx(hwnd, (HRGN)msg->wParam, DCX_WINDOW|DCX_INTERSECTRGN);
    //HBRUSH TransperrantBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    auto window_rect = RECT{};
    GetWindowRect(hwnd, &window_rect);
    //::FillRect(hdc, &window_rect, TransperrantBrush);
    //Rectangle(hdc, window_rect.left, window_rect.top, 
    //    window_rect.right, window_rect.bottom); 
    //ReleaseDC(hwnd, hdc);
    //if (IsGlass())
    //  return DefWindowProc(hwnd, message, w_param, l_param);
    //GetWindowRect(hwnd, &wr);
    //if (!msg->wParam || w_param == 1) {
    //  dirty = wr;
    //  dirty.left = dirty.top = 0;
    //} else {
    //  GetRgnBox(reinterpret_cast<HRGN>(msg->wParam), &dirty_box);
    //  if (!IntersectRect(&dirty, &dirty_box, &wr))
    //    return 0;
    //  OffsetRect(&dirty, -wr.left, -wr.top);
    //}
    //auto hdc = GetWindowDC(hwnd);
    ////auto br = CreateSolidBrush(RGB(255,0,0));
    //HBRUSH br = (HBRUSH)::GetStockObject(NULL_BRUSH);
    //FillRect(hdc, &window_rect, br);
    //DeleteObject(br);
    //ReleaseDC(hwnd, hdc);
    //return true;
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
      WS_THICKFRAME | WS_CAPTION | WS_EX_TRANSPARENT);
  } else if(m_is_resizable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
      WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | WS_EX_TRANSPARENT);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX | WS_CAPTION);
  }
  //DWMNCRENDERINGPOLICY ncrp = DWMNCRP_LAST;
  //// Disable non-client area rendering on the window.
  //::DwmSetWindowAttribute(hwnd,
  //    DWMWA_NCRENDERING_POLICY,
  //    &ncrp,
  //    sizeof(ncrp));
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
